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

#include "FormWidget.h"
#include "Form.h"
#include "Property.h"
#include "WidgetPropertyModel.h"
#include "WidgetRegistry.h"
#include <LibGUI/Button.h>
#include <LibGUI/CheckBox.h>
#include <LibGUI/GroupBox.h>
#include <LibGUI/Label.h>
#include <LibGUI/Painter.h>
#include <LibGUI/ProgressBar.h>
#include <LibGUI/RadioButton.h>
#include <LibGUI/ScrollBar.h>
#include <LibGUI/Slider.h>
#include <LibGUI/SpinBox.h>
#include <LibGUI/TextEditor.h>

namespace HackStudio {

FormWidget::FormWidget(WidgetType type, Form& form, FormWidget* parent)
    : m_type(type)
    , m_form(form)
    , m_property_model(WidgetPropertyModel::create(*this))
{
    auto* widget_parent = parent ? parent->gwidget() : &form;
    m_gwidget = WidgetRegistry::build_gwidget(*this, type, widget_parent, m_properties);
    m_form.m_gwidget_map.set(m_gwidget, this);
    setup_properties();
}

Gfx::IntRect FormWidget::rect() const
{
    auto* parent = m_gwidget->parent_widget();
    if (!parent) {
        return m_gwidget->rect();
    }

    auto relative_rect = m_gwidget->relative_rect();
    relative_rect.move_by(parent->relative_position());
    return relative_rect;
}

void FormWidget::set_rect(const Gfx::IntRect& rect_)
{
    auto current_rect = rect();
    if (rect_ == current_rect)
        return;
    m_gwidget->set_relative_rect(rect_);
    synchronize_properties();
}

bool FormWidget::is_selected() const
{
    return m_form.is_selected(*this);
}

Gfx::IntRect FormWidget::grabber_rect(Direction direction) const
{
    int grabber_size = 5;
    int half_grabber_size = grabber_size / 2;
    switch (direction) {
    case Direction::Left:
        return { rect().x() - half_grabber_size, rect().center().y() - half_grabber_size, grabber_size, grabber_size };
    case Direction::UpLeft:
        return { rect().x() - half_grabber_size, rect().y() - half_grabber_size, grabber_size, grabber_size };
    case Direction::Up:
        return { rect().center().x() - half_grabber_size, rect().y() - half_grabber_size, grabber_size, grabber_size };
    case Direction::UpRight:
        return { rect().right() - half_grabber_size, rect().y() - half_grabber_size, grabber_size, grabber_size };
    case Direction::Right:
        return { rect().right() - half_grabber_size, rect().center().y() - half_grabber_size, grabber_size, grabber_size };
    case Direction::DownLeft:
        return { rect().x() - half_grabber_size, rect().bottom() - half_grabber_size, grabber_size, grabber_size };
    case Direction::Down:
        return { rect().center().x() - half_grabber_size, rect().bottom() - half_grabber_size, grabber_size, grabber_size };
    case Direction::DownRight:
        return { rect().right() - half_grabber_size, rect().bottom() - half_grabber_size, grabber_size, grabber_size };
    default:
        ASSERT_NOT_REACHED();
    }
}

Direction FormWidget::grabber_at(const Gfx::IntPoint& position) const
{
    Direction found_grabber = Direction::None;
    for_each_direction([&](Direction direction) {
        if (grabber_rect(direction).contains(position))
            found_grabber = direction;
    });
    return found_grabber;
}

void FormWidget::for_each_property(Function<void(Property&)> callback)
{
    for (auto& it : m_properties) {
        callback(it);
    }
}

void FormWidget::add_property(const String& name, Function<GUI::Variant(const GUI::Widget&)>&& getter, Function<void(GUI::Widget&, const GUI::Variant&)>&& setter)
{
    auto& prop = property(name);
    prop.m_getter = move(getter);
    prop.m_setter = move(setter);
}

#define _ADD_PROPERTY(gclass, name, getter, setter, variant_type)                      \
    add_property(                                                                      \
        name,                                                                          \
        [](auto& widget) -> GUI::Variant { return ((const gclass&)widget).getter(); }, \
        [](auto& widget, auto& value) { ((gclass&)widget).setter(value.to_##variant_type()); })

void FormWidget::setup_properties()
{
    _ADD_PROPERTY(Core::Object, "name", name, set_name, string);

    _ADD_PROPERTY(GUI::Widget, "width", width, set_width, i32);
    _ADD_PROPERTY(GUI::Widget, "height", height, set_height, i32);
    _ADD_PROPERTY(GUI::Widget, "x", x, set_x, i32);
    _ADD_PROPERTY(GUI::Widget, "y", y, set_y, i32);
    _ADD_PROPERTY(GUI::Widget, "visible", is_visible, set_visible, bool);
    _ADD_PROPERTY(GUI::Widget, "enabled", is_enabled, set_enabled, bool);
    _ADD_PROPERTY(GUI::Widget, "tooltip", tooltip, set_tooltip, string);
    _ADD_PROPERTY(GUI::Widget, "backcolor", background_color, set_background_color, color);
    _ADD_PROPERTY(GUI::Widget, "forecolor", foreground_color, set_foreground_color, color);
    _ADD_PROPERTY(GUI::Widget, "autofill", fill_with_background_color, set_fill_with_background_color, bool);

    if (m_type == WidgetType::GLabel) {
        _ADD_PROPERTY(GUI::Label, "text", text, set_text, string);
    }

    if (m_type == WidgetType::GButton) {
        _ADD_PROPERTY(GUI::Button, "text", text, set_text, string);
    }

    if (m_type == WidgetType::GGroupBox) {
        _ADD_PROPERTY(GUI::GroupBox, "title", title, set_title, string);
    }

    if (m_type == WidgetType::GScrollBar) {
        _ADD_PROPERTY(GUI::ScrollBar, "min", min, set_min, i32);
        _ADD_PROPERTY(GUI::ScrollBar, "max", max, set_max, i32);
        _ADD_PROPERTY(GUI::ScrollBar, "value", value, set_value, i32);
        _ADD_PROPERTY(GUI::ScrollBar, "step", step, set_step, i32);
    }

    if (m_type == WidgetType::GSpinBox) {
        _ADD_PROPERTY(GUI::SpinBox, "min", min, set_min, i32);
        _ADD_PROPERTY(GUI::SpinBox, "max", max, set_max, i32);
        _ADD_PROPERTY(GUI::SpinBox, "value", value, set_value, i32);
    }

    if (m_type == WidgetType::GProgressBar) {
        _ADD_PROPERTY(GUI::ProgressBar, "min", min, set_min, i32);
        _ADD_PROPERTY(GUI::ProgressBar, "max", max, set_max, i32);
        _ADD_PROPERTY(GUI::ProgressBar, "value", value, set_value, i32);
    }

    if (m_type == WidgetType::GSlider) {
        _ADD_PROPERTY(GUI::Slider, "min", min, set_min, i32);
        _ADD_PROPERTY(GUI::Slider, "max", max, set_max, i32);
        _ADD_PROPERTY(GUI::Slider, "value", value, set_value, i32);
    }

    if (m_type == WidgetType::GTextEditor) {
        _ADD_PROPERTY(GUI::TextEditor, "text", text, set_text, string);
        _ADD_PROPERTY(GUI::TextEditor, "ruler_visible", is_ruler_visible, set_ruler_visible, bool);
    }

    if (m_type == WidgetType::GCheckBox) {
        _ADD_PROPERTY(GUI::CheckBox, "text", text, set_text, string);
        _ADD_PROPERTY(GUI::CheckBox, "checked", is_checked, set_checked, bool);
    }

    if (m_type == WidgetType::GRadioButton) {
        _ADD_PROPERTY(GUI::RadioButton, "text", text, set_text, string);
        _ADD_PROPERTY(GUI::RadioButton, "checked", is_checked, set_checked, bool);
    }
}

void FormWidget::synchronize_properties()
{
    for (auto& prop : m_properties) {
        if (prop.m_getter)
            prop.m_value = prop.m_getter(*gwidget());
    }

    m_property_model->update();
}

Property& FormWidget::property(const String& name)
{
    for (auto& prop : m_properties) {
        if (prop.name() == name)
            return prop;
    }
    m_properties.append(make<Property>(*this, name, GUI::Variant()));
    return m_properties.last();
}

void FormWidget::property_did_change()
{
    m_form.update();
}

void FormWidget::capture_transform_origin_rect()
{
    m_transform_origin_rect = rect();
}

bool FormWidget::is_in_layout() const
{
    if (auto* parent_widget = m_gwidget->parent_widget()) {
        if (parent_widget->layout())
            return true;
    }
    return false;
}

FormWidget::~FormWidget()
{
    m_form.m_gwidget_map.remove(m_gwidget);
    m_form.m_selected_widgets.remove(this);
    m_gwidget->parent()->remove_child(*m_gwidget);
}

}
