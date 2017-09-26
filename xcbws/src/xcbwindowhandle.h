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

#pragma once
#include <QObject>
#include <QList>
#include <memory>
#include <xcb/xproto.h>
#include "core/extension.h"
#include "core/queryhandler.h"
#include "core/item.h"
#include "core/action.h"

namespace XcbWS {

class not_applicable_t final : std::exception {
public:
    const char* what() const noexcept override { return "This cookie does not result in a valid handle"; }
};

class XcbWindowHandle final
{
public:

    XcbWindowHandle(xcb_window_t, xcb_get_property_cookie_t &);
    ~XcbWindowHandle();

    QString name() const;
    void raise();
    std::shared_ptr<Core::Item> produceItem();

    static std::vector<std::shared_ptr<XcbWindowHandle> > getRootChildren();

private:
    xcb_window_t window_;
    QString name_;

};
}
