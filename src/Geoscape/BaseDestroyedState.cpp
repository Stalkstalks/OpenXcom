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
#include "BaseDestroyedState.h"
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Engine/Surface.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/Region.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/Ufo.h"
#include "../Ruleset/RuleRegion.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

BaseDestroyedState::BaseDestroyedState(Game *game, Base *base) : State(game), _base(base)
{
	std::string background, backpalette;
	Uint8 colors[2];

	_screen = false;
	// Create objects
	_window = new Window(this, 256, 160, 32, 20);
	_btnOk = new TextButton(100, 20, 110, 142);
	_txtMessage = new Text(224, 48, 48, 76);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_BACK15.SCR";
		backpalette = "TFTD_BACKPALS.DAT";

		colors[0] = Palette::blockOffset(3);
		colors[1] = Palette::blockOffset(0)+1;
	}
	else
	{
		// Basic properties for display in UFO style
		background = "BACK15.SCR";
		backpalette = "BACKPALS.DAT";

		colors[0] = Palette::blockOffset(7);
		colors[1] = Palette::blockOffset(8)+5;
	}

	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);
	
	add(_window);
	add(_btnOk);
	add(_txtMessage);

	centerAllSurfaces();
	
	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnOk->setColor(colors[1]);
	_btnOk->setText(_game->getLanguage()->getString("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BaseDestroyedState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BaseDestroyedState::btnOkClick, (SDLKey)Options::getInt("keyOk"));
	_btnOk->onKeyboardPress((ActionHandler)&BaseDestroyedState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));
		
	_txtMessage->setAlign(ALIGN_CENTER);
	_txtMessage->setBig();
	_txtMessage->setWordWrap(true);
	_txtMessage->setColor(colors[1]);

	std::wstringstream ss;
	ss << _game->getLanguage()->getString("STR_THE_ALIENS_HAVE_DESTROYED_THE_UNDEFENDED_BASE") << _base->getName();
	_txtMessage->setText(ss.str());

	std::vector<Region*>::iterator k = _game->getSavedGame()->getRegions()->begin();
	for (; k != _game->getSavedGame()->getRegions()->end(); ++k)
	{
		if ((*k)->getRules()->insideRegion((base)->getLongitude(), (base)->getLatitude()))
		{
			break;
		}
	}

	AlienMission* am = _game->getSavedGame()->getAlienMission((*k)->getRules()->getType(), "STR_ALIEN_RETALIATION");
	for (std::vector<Ufo*>::iterator i = _game->getSavedGame()->getUfos()->begin(); i != _game->getSavedGame()->getUfos()->end();)
	{
		if ((*i)->getMission() == am)
		{
			delete *i;
			i = _game->getSavedGame()->getUfos()->erase(i);
		}
		else
		{
			++i;
		}
	}

	for (std::vector<AlienMission*>::iterator i = _game->getSavedGame()->getAlienMissions().begin();
		i != _game->getSavedGame()->getAlienMissions().end(); ++i)
	{
		if ((AlienMission*)(*i) == am)
		{
			delete (*i);
			_game->getSavedGame()->getAlienMissions().erase(i);
			break;
		}
	}
}

/**
 *
 */
BaseDestroyedState::~BaseDestroyedState()
{
}

/**
 * Resets the palette.
 */
void BaseDestroyedState::init()
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

		color = Palette::blockOffset(7);
	}

	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(color), Palette::backPos, 16);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BaseDestroyedState::btnOkClick(Action *)
{
	_game->popState();
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		if ((*i) == _base)
		{
			delete (*i);
			_game->getSavedGame()->getBases()->erase(i);
			break;
		}
	}
}

}
