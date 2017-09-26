// albert - a simple application launcher for linux
// Copyright (C) 2017 Martin Buergmann
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "xcbwindowhandle.h"
#include <xcb/xcb.h>
#include <QtX11Extras/QX11Info>

using ActionVector = std::vector<std::shared_ptr<Core::Action> >;

namespace {

using XcbWS::XcbWindowHandle;

class WindowItem : public Core::Item {
public:
    WindowItem(XcbWindowHandle& win) : win_(win) {}
    WindowItem(XcbWindowHandle* win) : win_(*win) {}
    QString id() const override { return ""; }
    QString iconPath() const override { return "go-top"; }
    QString text() const override { return win_.name(); }
    QString subtext() const override { return "Manipulate window"; }
    std::vector<std::shared_ptr<Core::Action>> actions() override;
private:
    XcbWindowHandle& win_;
};

class RaiseAction : public Core::Action {
public:
    RaiseAction(XcbWindowHandle& win);
    QString text() const override { return "Raise"; }
    void activate() { win_.raise(); }
private:
    XcbWindowHandle& win_;
};

}

/** ***************************************************************************/
XcbWS::XcbWindowHandle::XcbWindowHandle(xcb_window_t w, xcb_get_property_cookie_t cookie) : window_(w) {
    // Get the property reply from the cookie
    xcb_get_property_reply_t* prop = xcb_get_property_reply(QX11Info::connection(), cookie, NULL);

    // And skip invalid ones
    if (!prop)
        throw not_applicable_t();
    int len = xcb_get_property_value_length(prop);
    if (len == 0)
        throw not_applicable_t();

    // Extract the name
    char* nameloc = (char*) xcb_get_property_value(prop);
    name_ = (char*) malloc(len +1);
    memcpy(name_, nameloc, len);
    name_[len] = 0;

    free(prop);

}



/** ***************************************************************************/
XcbWS::XcbWindowHandle::~XcbWindowHandle() {
    free(name_);
}



/** ***************************************************************************/
QString XcbWS::XcbWindowHandle::name() const {
    return QString(name_);
}



/** ***************************************************************************/
void XcbWS::XcbWindowHandle::raise() {
    static const uint32_t values[] = {XCB_STACK_MODE_ABOVE};
    xcb_void_cookie_t check = xcb_configure_window(QX11Info::connection(), window_, XCB_CONFIG_WINDOW_STACK_MODE, values);

    xcb_generic_error_t* err;
    if ((err = xcb_request_check(QX11Info::connection(), check))) {
        qWarning("Raising window ended with error %d", err->error_code);
        free(err);
    }
}



/** ***************************************************************************/
std::shared_ptr<Core::Item> XcbWindowHandle::produceItem() {
    return std::shared_ptr<Core::Item>(new WindowItem(this));
}



/** ***************************************************************************/
std::vector<std::shared_ptr<XcbWS::XcbWindowHandle> > XcbWS::XcbWindowHandle::getRootChildren() {
    xcb_connection_t* xc = QX11Info::connection();

    /* Get the screen whose number is QX11Info::appScreen() */
    const xcb_setup_t *setup = xcb_get_setup (xc);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (setup);

    // we want the screen at index QX11Info::appScreen() of the iterator
    for (int i = 0; i < QX11Info::appScreen(); ++i) {
        xcb_screen_next (&iter);
    }

    xcb_screen_t *screen = iter.data;

    // From this screen the root window
    xcb_window_t rootWin = screen->root;

    // Query all child windows from the root window
    xcb_query_tree_cookie_t qtc = xcb_query_tree(xc, rootWin);
    xcb_query_tree_reply_t* tree = xcb_query_tree_reply(xc, qtc, NULL);
    xcb_window_t* children = xcb_query_tree_children(tree);
    int nChildren = xcb_query_tree_children_length(tree);

    // For every child window try to create a XcbWindowHandle
    xcb_get_property_cookie_t* requestCookies = (xcb_get_property_cookie_t*) malloc(sizeof(xcb_get_property_cookie_t) * nChildren);

    for (int i = 0; i < nChildren; i++) {
        requestCookies[i] = xcb_get_property(xc, false, children[i], XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 100);
    }


    std::vector<std::shared_ptr<XcbWindowHandle>> toReturn;
    for (int i = 0; i < nChildren; i++) {
        try {
            XcbWindowHandle* next = new XcbWindowHandle(children[i], requestCookies[i]);
            toReturn.emplace_back(next);
        } catch(not_applicable_t) {
            continue;
        }
    }
    return toReturn;
}



/** ***************************************************************************/
ActionVector WindowItem::actions() {
    ActionVector ret;
    ret.emplace_back(new RaiseAction(win_));
    return ret;
}
