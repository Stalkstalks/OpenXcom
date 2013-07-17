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
#include "AbandonGameState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/SavedGame.h"
#include "../Menu/MainMenuState.h"
#include "../Engine/Options.h"
#include "../Menu/SaveState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Abandon Game screen.
 * @param game Pointer to the core game.
 */
AbandonGameState::AbandonGameState(Game *game) : State(game)
{
	std::string background, backpalette;
	Uint8 colors[2];

	_screen = false;

	// Create objects
	_window = new Window(this, 216, 160, 20, 20, POPUP_BOTH);
	_btnYes = new TextButton(50, 20, 38, 140);
	_btnNo = new TextButton(50, 20, 168, 140);
	_txtTitle = new Text(206, 15, 25, 70);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_BACK01.SCR";
		backpalette = "TFTD_BACKPALS.DAT";

		colors[0] = Palette::blockOffset(4);
		colors[1] = Palette::blockOffset(0)+1;
	}
	else
	{
		// Basic properties for display in UFO style
		background = "BACK01.SCR";
		backpalette = "BACKPALS.DAT";

		colors[0] = Palette::blockOffset(0);
		colors[1] = Palette::blockOffset(15)-1;
	}

	// Set palette
	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);

	add(_window);
	add(_btnYes);
	add(_btnNo);
	add(_txtTitle);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnYes->setColor(colors[1]);
	_btnYes->setText(_game->getLanguage()->getString("STR_YES"));
	_btnYes->onMouseClick((ActionHandler)&AbandonGameState::btnYesClick);
	_btnYes->onKeyboardPress((ActionHandler)&AbandonGameState::btnYesClick, (SDLKey)Options::getInt("keyOk"));

	_btnNo->setColor(colors[1]);
	_btnNo->setText(_game->getLanguage()->getString("STR_NO"));
	_btnNo->onMouseClick((ActionHandler)&AbandonGameState::btnNoClick);
	_btnNo->onKeyboardPress((ActionHandler)&AbandonGameState::btnNoClick, (SDLKey)Options::getInt("keyCancel"));

	_txtTitle->setColor(colors[1]);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	std::wstringstream ss;
	ss << _game->getLanguage()->getString("STR_ABANDON_GAME_QUESTION");
	_txtTitle->setText(ss.str());
}

/**
 *
 */
AbandonGameState::~AbandonGameState()
{

}

/**
 * Goes back to the Main Menu.
 * @param action Pointer to an action.
 */
void AbandonGameState::btnYesClick(Action *)
{
	if (Options::getInt("autosave") == 3)
	{
		SaveState *ss = new SaveState(_game, true, false);
		delete ss;
	}

	_game->setState(new MainMenuState(_game));
	_game->setSavedGame(0);
}

/**
 * Closes the window.
 * @param action Pointer to an action.
 */
void AbandonGameState::btnNoClick(Action *)
{
	_game->popState();
}

}
