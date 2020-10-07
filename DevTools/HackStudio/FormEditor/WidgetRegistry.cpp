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

#include "WidgetRegistry.h"
#include "Property.h"
#include <LibGUI/Button.h>
#include <LibGUI/CheckBox.h>
#include <LibGUI/GroupBox.h>
#include <LibGUI/Label.h>
#include <LibGUI/ProgressBar.h>
#include <LibGUI/RadioButton.h>
#include <LibGUI/ScrollBar.h>
#include <LibGUI/Slider.h>
#include <LibGUI/SpinBox.h>
#include <LibGUI/TextBox.h>
#include <LibGUI/TextEditor.h>

namespace HackStudio {

String to_class_name(WidgetType type)
{
    switch (type) {
    case WidgetType::GWidget:
        return "GUI::Widget";
    case WidgetType::GButton:
        return "GButton";
    case WidgetType::GLabel:
        return "GLabel";
    case WidgetType::GSpinBox:
        return "GSpinBox";
    case WidgetType::GTextBox:
        return "GTextBox";
    case WidgetType::GProgressBar:
        return "GProgressBar";
    case WidgetType::GCheckBox:
        return "GCheckBox";
    case WidgetType::GRadioButton:
        return "GRadioButton";
    case WidgetType::GScrollBar:
        return "GScrollBar";
    case WidgetType::GGroupBox:
        return "GGroupBox";
    case WidgetType::GSlider:
        return "GSlider";
    default:
        ASSERT_NOT_REACHED();
    }
}

WidgetType widget_type_from_class_name(const StringView& name)
{
    if (name == "GUI::Widget")
        return WidgetType::GWidget;
    if (name == "GUI::Button")
        return WidgetType::GButton;
    if (name == "GUI::Label")
        return WidgetType::GLabel;
    if (name == "GUI::SpinBox")
        return WidgetType::GSpinBox;
    if (name == "GUI::TextEditor")
        return WidgetType::GTextEditor;
    if (name == "GUI::TextBox")
        return WidgetType::GTextBox;
    if (name == "GUI::ProgressBar")
        return WidgetType::GProgressBar;
    if (name == "GUI::CheckBox")
        return WidgetType::GCheckBox;
    if (name == "GUI::RadioButton")
        return WidgetType::GRadioButton;
    if (name == "GUI::ScrollBar")
        return WidgetType::GScrollBar;
    if (name == "GUI::GroupBox")
        return WidgetType::GGroupBox;
    if (name == "GUI::Slider")
        return WidgetType::GSlider;
    return WidgetType::None;
}

static RefPtr<GUI::Widget> build_gwidget(WidgetType type, GUI::Widget* parent)
{
    switch (type) {
    case WidgetType::GWidget:
        return parent->add<GUI::Widget>();
    case WidgetType::GScrollBar:
        return parent->add<GUI::ScrollBar>(Orientation::Vertical);
    case WidgetType::GGroupBox:
        return parent->add<GUI::GroupBox>("groupbox_1");
    case WidgetType::GLabel: {
        auto& label = parent->add<GUI::Label>();
        label.set_fill_with_background_color(true);
        label.set_text("label_1");
        return label;
    }
    case WidgetType::GButton: {
        auto& button = parent->add<GUI::Button>();
        button.set_text("button_1");
        return button;
    }
    case WidgetType::GSpinBox: {
        auto& box = parent->add<GUI::SpinBox>();
        box.set_range(0, 100);
        box.set_value(0);
        return box;
    }
    case WidgetType::GTextBox: {
        auto& textbox = parent->add<GUI::TextBox>();
        textbox.set_text("text_1");
        return textbox;
    }
    case WidgetType::GTextEditor: {
        auto& editor = parent->add<GUI::TextEditor>();
        editor.set_ruler_visible(false);
        return editor;
    }
    case WidgetType::GProgressBar: {
        auto& bar = parent->add<GUI::ProgressBar>();
        bar.set_format(GUI::ProgressBar::Format::NoText);
        bar.set_range(0, 100);
        bar.set_value(50);
        return bar;
    }
    case WidgetType::GSlider: {
        auto& slider = parent->add<GUI::HorizontalSlider>();
        slider.set_range(0, 100);
        slider.set_value(50);
        return slider;
    }
    case WidgetType::GCheckBox: {
        auto& box = parent->add<GUI::CheckBox>();
        box.set_text("checkbox_1");
        return box;
    }
    case WidgetType::GRadioButton:
        return parent->add<GUI::RadioButton>("radio_1");
    default:
        ASSERT_NOT_REACHED();
        return nullptr;
    }
}

RefPtr<GUI::Widget> WidgetRegistry::build_gwidget(HackStudio::FormWidget& widget, WidgetType type, GUI::Widget* parent, NonnullOwnPtrVector<Property>& properties)
{
    auto gwidget = HackStudio::build_gwidget(type, parent);
    auto property = make<Property>(widget, "class", to_class_name(type));
    property->set_readonly(true);
    properties.append(move(property));
    return gwidget;
}

}
