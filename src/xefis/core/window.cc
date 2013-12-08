/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QShortcut>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QLabel>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/panel.h>
#include <xefis/core/config_reader.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/components/configurator/configurator_widget.h>
#include <xefis/widgets/panel_button.h>
#include <xefis/widgets/panel_rotary_encoder.h>
#include <xefis/widgets/panel_numeric_display.h>

// Local:
#include "window.h"


namespace Xefis {

Window::Window (Application* application, ConfigReader* config_reader, QDomElement const& element):
	_application (application),
	_config_reader (config_reader)
{
	setWindowTitle ("XEFIS");
	resize (limit (element.attribute ("width").toInt(), 40, 10000),
			limit (element.attribute ("height").toInt(), 30, 10000));
	setMouseTracking (true);
	setAttribute (Qt::WA_TransparentForMouseEvents);

	if (element.attribute ("full-screen") == "true")
		setWindowState (windowState() | Qt::WindowFullScreen);

	_stack = new QStackedWidget (this);

	_instruments_panel = new QWidget (_stack);
	_instruments_panel->setBackgroundRole (QPalette::Shadow);
	_instruments_panel->setAutoFillBackground (true);
	// Black background:
	QPalette p = palette();
	p.setColor (QPalette::Shadow, Qt::black);
	p.setColor (QPalette::Dark, Qt::gray);
	_instruments_panel->setPalette (p);

	_configurator_panel = new QWidget (this);

	QLayout* configurator_layout = new QVBoxLayout (_configurator_panel);
	configurator_layout->setMargin (WidgetMargin);
	configurator_layout->setSpacing (0);

	QBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_stack);

	_stack->addWidget (_instruments_panel);
	_stack->addWidget (_configurator_panel);
	_stack->setCurrentWidget (_instruments_panel);

	new QShortcut (Qt::Key_Escape, this, SLOT (show_configurator()));

	process_window_element (element);
}


void
Window::data_updated (Time const&)
{
	for (auto stack: _stacks)
	{
		if (stack->property.fresh() && stack->property.valid())
			stack->layout->setCurrentIndex (*stack->property);
	}
}


float
Window::pen_scale() const
{
	return _application->config_reader()->pen_scale();
}


float
Window::font_scale() const
{
	return _application->config_reader()->font_scale();
}


void
Window::process_window_element (QDomElement const& window_element)
{
	for (QDomElement& e: window_element)
	{
		if (e == "layout")
		{
			QLayout* layout = process_layout_element (e, _instruments_panel, nullptr);
			if (_instruments_panel->layout())
				throw Exception ("a window can only have one layout");
			_instruments_panel->setLayout (layout);
		}
		else
			throw Exception (QString ("unsupported child of <window>: <%1>").arg (e.tagName()).toStdString());
	}
}


Panel*
Window::process_panel_element (QDomElement const& panel_element, QWidget* parent_widget)
{
	Panel* panel = new Panel (parent_widget, _application);

	for (QDomElement& e: panel_element)
	{
		if (e == "layout")
		{
			QLayout* layout = process_layout_element (e, panel, panel);
			layout->setMargin (5);
			if (panel->layout())
				throw Exception ("a panel can only have one layout");
			panel->setLayout (layout);
		}
		else
			throw Exception (QString ("unsupported child of <panel>: <%1>").arg (e.tagName()).toStdString());
	}

	return panel;
}


QGroupBox*
Window::process_group_element (QDomElement const& group_element, QWidget* parent_widget, Panel* panel)
{
	if (!panel)
		throw Exception ("<group> can only be used as descendant of <panel>");

	QGroupBox* group = new QGroupBox (group_element.attribute ("label").replace ("&", "&&"), parent_widget);

	for (QDomElement& e: group_element)
	{
		if (e == "layout")
		{
			QLayout* layout = process_layout_element (e, group, panel);
			layout->setMargin (5);
			if (group->layout())
				throw Exception ("a group can only have one layout");
			group->setLayout (layout);
		}
		else
			throw Exception (QString ("unsupported child of <group>: <%1>").arg (e.tagName()).toStdString());
	}

	return group;
}


QWidget*
Window::process_widget_element (QDomElement const& widget_element, QWidget* parent_widget, Panel* panel)
{
	if (!panel)
		throw Exception ("<widget> can only be used as descendant of <panel>");

	QString type = widget_element.attribute ("type");
	QWidget* widget = nullptr;
	QWidget* label_wrapper = nullptr;
	QLabel* top_label = nullptr;
	QLabel* bottom_label = nullptr;

	QString top_label_str = widget_element.attribute ("top-label");
	QString bottom_label_str = widget_element.attribute ("bottom-label");

	if (!top_label_str.isEmpty() || !bottom_label_str.isEmpty())
	{
		label_wrapper = new PanelWidget (parent_widget, panel);

		if (!top_label_str.isEmpty())
		{
			top_label = new QLabel (top_label_str, label_wrapper);
			top_label->setTextFormat (Qt::PlainText);
			top_label->setAlignment (Qt::AlignHCenter | Qt::AlignBottom);
			top_label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		}

		if (!bottom_label_str.isEmpty())
		{
			bottom_label = new QLabel (bottom_label_str, label_wrapper);
			bottom_label->setTextFormat (Qt::PlainText);
			bottom_label->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
			bottom_label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		}
	}

	if (type == "button")
	{
		PanelButton::LEDColor color = PanelButton::Green;
		if (widget_element.attribute ("led-color") == "amber")
			color = PanelButton::Amber;

		widget = new PanelButton (parent_widget, panel, color,
								  PropertyBoolean (widget_element.attribute ("click-property").toStdString()),
								  PropertyBoolean (widget_element.attribute ("led-property").toStdString()));
	}
	else if (type == "rotary-encoder")
	{
		widget = new PanelRotaryEncoder (parent_widget, panel,
										 widget_element.attribute ("click-label"),
										 PropertyBoolean (widget_element.attribute ("rotate-a-property").toStdString()),
										 PropertyBoolean (widget_element.attribute ("rotate-b-property").toStdString()),
										 PropertyBoolean (widget_element.attribute ("click-property").toStdString()));
	}
	else if (type == "numeric-display")
	{
		widget = new PanelNumericDisplay (parent_widget, panel,
										  widget_element.attribute ("digits").toUInt(),
										  widget_element.attribute ("pad-with-zeros") == "true",
										  PropertyInteger (widget_element.attribute ("value-property").toStdString()));
	}
	else
		throw Exception (QString ("unsupported widget type '%1'").arg (type));

	if (label_wrapper)
	{
		QVBoxLayout* layout = new QVBoxLayout (label_wrapper);
		layout->setMargin (0);
		layout->setSpacing (2);
		layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		if (top_label)
			layout->addWidget (top_label, 0, Qt::AlignCenter);
		layout->addWidget (widget, 0, Qt::AlignCenter);
		if (bottom_label)
			layout->addWidget (bottom_label, 0, Qt::AlignCenter);
		layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		return label_wrapper;
	}
	else
		return widget;
}


QLayout*
Window::process_layout_element (QDomElement const& layout_element, QWidget* parent_widget, Panel* panel)
{
	QLayout* new_layout = nullptr;
	QBoxLayout* box_new_layout = nullptr;
	Shared<Stack> stack;

	QString type = layout_element.attribute ("type");
	unsigned int spacing = layout_element.attribute ("spacing").toUInt();

	if (type == "horizontal")
		new_layout = box_new_layout = new QHBoxLayout();
	else if (type == "vertical")
		new_layout = box_new_layout = new QVBoxLayout();
	else if (type == "stack")
	{
		if (!layout_element.hasAttribute ("path"))
			throw Exception ("missing @path attribute on <layout type='stack'>");

		stack = std::make_shared<Stack>();
		stack->property.set_path (layout_element.attribute ("path").toStdString());
		stack->property.set_default (0);
		new_layout = stack->layout = new QStackedLayout();
		_stacks.insert (stack);
	}
	else
		throw Exception ("layout type must be 'vertical', 'horizontal' or 'stack'");

	new_layout->setSpacing (0);
	new_layout->setMargin (0);

	if (box_new_layout)
		box_new_layout->setSpacing (spacing);

	for (QDomElement& e: layout_element)
	{
		if (e == "item")
			process_item_element (e, new_layout, parent_widget, panel, stack);
		else if (e == "stretch")
		{
			if (!panel)
				throw Exception ("<stretch> can only be used as descendant of <panel>");

			if (box_new_layout)
				box_new_layout->addStretch (10000);
		}
		else if (e == "separator")
		{
			if (type == "stack")
				throw Exception ("<separator> not allowed in stack-type layout");

			QWidget* separator = new QWidget (parent_widget);
			separator->setMinimumSize (2, 2);
			separator->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			separator->setBackgroundRole (QPalette::Dark);
			separator->setAutoFillBackground (true);
			separator->setCursor (QCursor (Qt::CrossCursor));
			new_layout->addWidget (separator);
		}
		else
			throw Exception (QString ("unsupported child of <layout>: <%1>").arg (e.tagName()));
	}

	return new_layout;
}


void
Window::process_item_element (QDomElement const& item_element, QLayout* layout, QWidget* parent_widget, Panel* panel, Shared<Stack> stack)
{
	QBoxLayout* box_layout = dynamic_cast<QBoxLayout*> (layout);
	QStackedLayout* stacked_layout = dynamic_cast<QStackedLayout*> (layout);

	assert (stacked_layout && stack);

	if (stacked_layout && item_element.hasAttribute ("stretch-factor"))
		throw Exception ("attribute @stretch-factor not allowed on <item> of stack-type layout");

	if (box_layout && item_element.hasAttribute ("id"))
		throw Exception ("attribute @id not allowed on <item> of non-stack-type layout");

	bool has_child = false;
	int stretch = limit (item_element.attribute ("stretch-factor").toInt(), 1, std::numeric_limits<int>::max());

	// <item>'s children:
	for (QDomElement& e: item_element)
	{
		if (has_child)
			throw Exception ("only one child element per <item> allowed");

		has_child = true;

		if (e == "layout")
		{
			if (box_layout)
			{
				QLayout* sub_layout = process_layout_element (e, parent_widget, panel);
				box_layout->addLayout (sub_layout, stretch);
			}
			else if (stacked_layout)
			{
				QLayout* sub_layout = process_layout_element (e, parent_widget, panel);
				QWidget* proxy_widget = new QWidget (parent_widget);
				proxy_widget->setLayout (sub_layout);
				stacked_layout->addWidget (proxy_widget);
				stacked_layout->setCurrentWidget (proxy_widget);
			}
		}
		else if (e == "instrument" || e == "panel" || e == "group" || e == "widget")
		{
			QWidget* widget = nullptr;

			if (e == "instrument")
			{
				Module* module = _config_reader->process_module_element (e, parent_widget);
				if (module)
					widget = dynamic_cast<QWidget*> (module);
			}
			else if (e == "panel")
				widget = process_panel_element (e, parent_widget);
			else if (e == "group")
				widget = process_group_element (e, parent_widget, panel);
			else if (e == "widget")
				widget = process_widget_element (e, parent_widget, panel);

			// If it's a widget, add it to layout:
			if (widget)
			{
				if (box_layout)
					box_layout->addWidget (widget, stretch);
				else if (stacked_layout)
				{
					stacked_layout->addWidget (widget);
					stacked_layout->setCurrentWidget (widget);
				}
			}
		}
		else
			throw Exception (QString ("unsupported child of <item>: <%1>").arg (e.tagName()).toStdString());
	}

	if (!has_child)
	{
		if (box_layout)
		{
			box_layout->addStretch (stretch);
			QWidget* p = box_layout->parentWidget();
			if (p)
				p->setCursor (QCursor (Qt::CrossCursor));
		}
		else if (stacked_layout)
		{
			// Empty widget:
			QWidget* empty_widget = new QWidget (parent_widget);
			empty_widget->setCursor (QCursor (Qt::CrossCursor));
			stacked_layout->addWidget (empty_widget);
			stacked_layout->setCurrentWidget (empty_widget);
		}
	}
}


void
Window::show_configurator()
{
	if (_stack->currentWidget() == _instruments_panel)
	{
		ConfiguratorWidget* configurator_widget = _application->configurator_widget();
		if (!configurator_widget)
			return;
		if (configurator_widget->owning_window())
			configurator_widget->owning_window()->configurator_taken();
		_configurator_panel->layout()->addWidget (configurator_widget);
		_stack->setCurrentWidget (_configurator_panel);
		configurator_widget->set_owning_window (this);
	}
	else
		_stack->setCurrentWidget (_instruments_panel);
}


void
Window::configurator_taken()
{
	_stack->setCurrentWidget (_instruments_panel);
}

} // namespace Xefis

