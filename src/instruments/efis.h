/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__INSTRUMENTS__EFIS_H__INCLUDED
#define XEFIS__INSTRUMENTS__EFIS_H__INCLUDED

// Qt:
#include <QtGui/QWidget>
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/instrument.h>
#include <widgets/efis_widget.h>
#include <widgets/efis_nav_widget.h>


class EFIS: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	EFIS (QWidget* parent);

	void
	set_path (QString const& path) override;

  public slots:
	/**
	 * Force EFIS to read data from properties.
	 */
	void
	read();

  private:
	EFISWidget*				_efis_widget		= nullptr;
	EFISNavWidget*			_efis_nav_widget	= nullptr;
	QUdpSocket*				_input				= nullptr;
	std::string				_property_path;

	Xefis::Property<float>	_ias_kt;
	Xefis::Property<float>	_ias_tendency_kt;
	Xefis::Property<float>	_minimum_ias_kt;
	Xefis::Property<float>	_maximum_ias_kt;
	Xefis::Property<float>	_gs_kt;
	Xefis::Property<float>	_tas_kt;
	Xefis::Property<float>	_mach;
	Xefis::Property<float>	_pitch_deg;
	Xefis::Property<float>	_roll_deg;
	Xefis::Property<float>	_heading_deg;
	Xefis::Property<float>	_fpm_alpha_deg;
	Xefis::Property<float>	_fpm_beta_deg;
	Xefis::Property<float>	_track_deg;
	Xefis::Property<float>	_altitude_ft;
	Xefis::Property<float>	_altitude_agl_ft;
	Xefis::Property<float>	_landing_altitude_ft;
	Xefis::Property<float>	_pressure_inhg;
	Xefis::Property<float>	_cbr_fpm;
	Xefis::Property<float>	_autopilot_alt_setting_ft;
	Xefis::Property<float>	_autopilot_speed_setting_kt;
	Xefis::Property<float>	_autopilot_heading_setting_deg;
	Xefis::Property<float>	_autopilot_cbr_setting_fpm;
	Xefis::Property<float>	_flight_director_pitch_deg;
	Xefis::Property<float>	_flight_director_roll_deg;
	Xefis::Property<bool>	_navigation_needles_enabled;
	Xefis::Property<float>	_navigation_gs_needle;
	Xefis::Property<float>	_navigation_hd_needle;
	Xefis::Property<float>	_dme_distance_nm;
};

#endif
