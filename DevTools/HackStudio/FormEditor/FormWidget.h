/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "WidgetType.h"
#include <AK/Function.h>
#include <AK/HashMap.h>
#include <AK/NonnullOwnPtrVector.h>
#include <AK/NonnullRefPtr.h>
#include <AK/RefCounted.h>
#include <AK/Weakable.h>
#include <LibGUI/Widget.h>
#include <LibGfx/Rect.h>

namespace HackStudio {

class Form;
class Property;
class WidgetPropertyModel;

enum class Direction {
    None,
    Left,
    UpLeft,
    Up,
    UpRight,
    Right,
    DownRight,
    Down,
    DownLeft
};
template<typename Callback>
inline void for_each_direction(Callback callback)
{
    callback(Direction::Left);
    callback(Direction::UpLeft);
    callback(Direction::Up);
    callback(Direction::UpRight);
    callback(Direction::Right);
    callback(Direction::DownRight);
    callback(Direction::Down);
    callback(Direction::DownLeft);
}

class FormWidget : public RefCounted<FormWidget>
    , public Weakable<FormWidget> {
    friend class WidgetPropertyModel;

public:
    static NonnullRefPtr<FormWidget> create(WidgetType type, Form& form, FormWidget* parent) { return adopt(*new FormWidget(type, form, parent)); }
    ~FormWidget();

    bool is_selected() const;

    Gfx::IntRect rect() const;
    void set_rect(const Gfx::IntRect&);

    Gfx::IntRect grabber_rect(Direction) const;
    Direction grabber_at(const Gfx::IntPoint&) const;

    GUI::Widget* gwidget() { return m_gwidget; }

    Property& property(const String&);

    void for_each_property(Function<void(Property&)>);

    WidgetPropertyModel& property_model() { return *m_property_model; }

    void setup_properties();
    void synchronize_properties();

    void property_did_change();

    Gfx::IntRect transform_origin_rect() const { return m_transform_origin_rect; }
    void capture_transform_origin_rect();

    bool is_in_layout() const;

private:
    FormWidget(WidgetType, Form&, FormWidget* parent);

    void add_property(const String& name, Function<GUI::Variant(const GUI::Widget&)>&& getter, Function<void(GUI::Widget&, const GUI::Variant&)>&& setter);

    WidgetType m_type { WidgetType::None };
    Form& m_form;
    RefPtr<GUI::Widget> m_gwidget;
    NonnullOwnPtrVector<Property> m_properties;
    NonnullRefPtr<WidgetPropertyModel> m_property_model;
    Gfx::IntRect m_transform_origin_rect;
};

}
