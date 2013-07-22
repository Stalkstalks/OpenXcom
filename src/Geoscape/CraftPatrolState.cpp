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
#include "CraftPatrolState.h"
#include <string>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/Craft.h"
#include "../Savegame/Target.h"
#include "GeoscapeCraftState.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Craft Patrol window.
 * @param game Pointer to the core game.
 * @param craft Pointer to the craft to display.
 * @param globe Pointer to the Geoscape globe.
 */
CraftPatrolState::CraftPatrolState(Game *game, Craft *craft, Globe *globe) : State(game), _craft(craft), _globe(globe)
{
	std::string background, backpalette;
	Uint8 colors[3];

	_screen = false;

	// Create objects
	_window = new Window(this, 224, 168, 16, 16, POPUP_BOTH);
	_btnOk = new TextButton(140, 12, 58, 144);
	_btnRedirect = new TextButton(140, 12, 58, 160);
	_txtDestination = new Text(224, 64, 16, 48);
	_txtPatrolling = new Text(224, 16, 16, 120);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_BACK12.SCR";
		backpalette = "TFTD_BACKPALS.DAT";

		colors[0] = Palette::blockOffset(4);
		colors[1] = colors[2] = Palette::blockOffset(0)+1;
	}
	else
	{
		// Basic properties for display in UFO style
		background = "BACK12.SCR";
		backpalette = "BACKPALS.DAT";

		colors[0] = Palette::blockOffset(4);
		colors[1] = Palette::blockOffset(15)-1;
		colors[2] = Palette::blockOffset(8)+5;
	}

	// Set palette
	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);

	add(_window);
	add(_btnOk);
	add(_btnRedirect);
	add(_txtDestination);
	add(_txtPatrolling);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnOk->setColor(colors[2]);
	_btnOk->setText(_game->getLanguage()->getString("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CraftPatrolState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CraftPatrolState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));

	_btnRedirect->setColor(colors[2]);
	_btnRedirect->setText(_game->getLanguage()->getString("STR_REDIRECT_CRAFT"));
	_btnRedirect->onMouseClick((ActionHandler)&CraftPatrolState::btnRedirectClick);
	_btnRedirect->onKeyboardPress((ActionHandler)&CraftPatrolState::btnRedirectClick, (SDLKey)Options::getInt("keyOk"));

	_txtDestination->setColor(colors[1]);
	_txtDestination->setBig();
	_txtDestination->setAlign(ALIGN_CENTER);
	_txtDestination->setWordWrap(true);
	std::wstringstream s;
	s <<_craft->getName(_game->getLanguage()) << L'\n';
	s << _game->getLanguage()->getString("STR_HAS_REACHED") << L'\n';
	s << _game->getLanguage()->getString("STR_DESTINATION") << L'\n';
	s << _craft->getDestination()->getName(_game->getLanguage());
	_txtDestination->setText(s.str());

	_txtPatrolling->setColor(colors[1]);
	_txtPatrolling->setBig();
	_txtPatrolling->setAlign(ALIGN_CENTER);
	_txtPatrolling->setText(_game->getLanguage()->getString("STR_NOW_PATROLLING"));
}

/**
 *
 */
CraftPatrolState::~CraftPatrolState()
{

}

/**
 * Resets the palette.
 */
void CraftPatrolState::init()
{
	std::string backpalette;
	Uint8 color;

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		backpalette = "TFTD_BACKPALS.DAT";

		color = Palette::blockOffset(4);
	}
	else
	{
		// Basic properties for display in UFO style
		backpalette = "BACKPALS.DAT";

		color = Palette::blockOffset(4);
	}

	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(color), Palette::backPos, 16);
}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void CraftPatrolState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Opens up the Craft window.
 * @param action Pointer to an action.
 */
void CraftPatrolState::btnRedirectClick(Action *)
{
	_game->popState();
	_game->pushState(new GeoscapeCraftState(_game, _craft, _globe, 0));
}

}
