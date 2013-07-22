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
#include "CraftErrorState.h"
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "GeoscapeState.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in a Cannot Rearm window.
 * @param game Pointer to the core game.
 * @param state Pointer to the Geoscape state.
 * @param msg Error message.
 */
CraftErrorState::CraftErrorState(Game *game, GeoscapeState *state, const std::wstring &msg) : State(game), _state(state)
{
	std::string background, backpalette;
	Uint8 colors[3];

	_screen = false;

	// Create objects
	_window = new Window(this, 256, 160, 32, 20, POPUP_BOTH);
	_btnOk = new TextButton(100, 18, 48, 150);
	_btnOk5Secs = new TextButton(100, 18, 172, 150);
	_txtMessage = new Text(226, 80, 47, 50);

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
	add(_btnOk5Secs);
	add(_txtMessage);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnOk->setColor(colors[2]);
	_btnOk->setText(_game->getLanguage()->getString("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CraftErrorState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CraftErrorState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));

	_btnOk5Secs->setColor(colors[2]);
	_btnOk5Secs->setText(_game->getLanguage()->getString("STR_OK_5_SECS"));
	_btnOk5Secs->onMouseClick((ActionHandler)&CraftErrorState::btnOk5SecsClick);
	_btnOk5Secs->onKeyboardPress((ActionHandler)&CraftErrorState::btnOk5SecsClick, (SDLKey)Options::getInt("keyOk"));

	_txtMessage->setColor(colors[1]);
	_txtMessage->setAlign(ALIGN_CENTER);
	_txtMessage->setVerticalAlign(ALIGN_MIDDLE);
	_txtMessage->setBig();
	_txtMessage->setWordWrap(true);
	_txtMessage->setText(msg);
}

/**
 *
 */
CraftErrorState::~CraftErrorState()
{

}

/**
 * Resets the palette.
 */
void CraftErrorState::init()
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
void CraftErrorState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void CraftErrorState::btnOk5SecsClick(Action *)
{
	_state->timerReset();
	_game->popState();
}

}
