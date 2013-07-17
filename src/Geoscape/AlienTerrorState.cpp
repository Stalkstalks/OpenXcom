/*
 * Copyright 2010-2013 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "AlienTerrorState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "GeoscapeState.h"
#include "Globe.h"
#include "../Savegame/SavedGame.h"
#include "../Ruleset/City.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Aliens Terrorise window.
 * @param game Pointer to the core game.
 * @param city Pointer to the city to get info from.
 * @param state Pointer to the Geoscape.
 */
AlienTerrorState::AlienTerrorState(Game *game, const City *city, GeoscapeState *state) : State(game), _city(city), _state(state)
{
	std::string background, backpalette;
	Uint8 colors[2];

	_screen = false;

	// Create objects
	_window = new Window(this, 256, 200, 0, 0, POPUP_BOTH);
	_btnCentre = new TextButton(200, 16, 28, 140);
	_btnCancel = new TextButton(200, 16, 28, 160);
	_txtTitle = new Text(246, 32, 5, 48);
	_txtCity = new Text(246, 16, 5, 80);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_BACK13.SCR";
		backpalette = "TFTD_BACKPALS.DAT";

		colors[0] = Palette::blockOffset(3);
		colors[1] = Palette::blockOffset(0)+1;
	}
	else
	{
		// Basic properties for display in UFO style
		background = "BACK03.SCR";
		backpalette = "BACKPALS.DAT";

		colors[0] = Palette::blockOffset(3);
		colors[1] = Palette::blockOffset(8)+5;
	}

	// Set palette
	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);

	add(_window);
	add(_btnCentre);
	add(_btnCancel);
	add(_txtTitle);
	add(_txtCity);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnCentre->setColor(colors[1]);
	_btnCentre->setText(_game->getLanguage()->getString("STR_CENTER_ON_SITE_TIME_5_SECS"));
	_btnCentre->onMouseClick((ActionHandler)&AlienTerrorState::btnCentreClick);
	_btnCentre->onKeyboardPress((ActionHandler)&AlienTerrorState::btnCentreClick, (SDLKey)Options::getInt("keyOk"));

	_btnCancel->setColor(colors[1]);
	_btnCancel->setText(_game->getLanguage()->getString("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&AlienTerrorState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&AlienTerrorState::btnCancelClick, (SDLKey)Options::getInt("keyCancel"));

	_txtTitle->setColor(colors[1]);
	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setWordWrap(true);
	_txtTitle->setText(_game->getLanguage()->getString("STR_ALIENS_TERRORISE"));

	_txtCity->setColor(colors[1]);
	_txtCity->setBig();
	_txtCity->setAlign(ALIGN_CENTER);
	_txtCity->setText(_game->getLanguage()->getString(_city->getName()));
}

/**
 *
 */
AlienTerrorState::~AlienTerrorState()
{

}

/**
 * Resets the palette.
 */
void AlienTerrorState::init()
{
	std::string backpalette;
	Uint8 color;

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		backpalette = "TFTD_BACKPALS.DAT";

		color = Palette::blockOffset(3);
	}
	else
	{
		// Basic properties for display in UFO style
		backpalette = "BACKPALS.DAT";

		color = Palette::blockOffset(3);
	}

	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(color), Palette::backPos, 16);
}

/**
 * Centers on the UFO and returns to the previous screen.
 * @param action Pointer to an action.
 */
void AlienTerrorState::btnCentreClick(Action *)
{
	_state->timerReset();
	_state->getGlobe()->center(_city->getLongitude(), _city->getLatitude());
	_game->popState();
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void AlienTerrorState::btnCancelClick(Action *)
{
	_game->popState();
}

}
