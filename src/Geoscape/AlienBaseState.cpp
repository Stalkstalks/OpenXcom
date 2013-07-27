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
#include "AlienBaseState.h"
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
#include "../Savegame/Region.h"
#include "../Ruleset/RuleRegion.h"
#include "../Savegame/AlienBase.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Aliens Base discovered window.
 * @param game Pointer to the core game.
 * @param base Pointer to the alien base to get info from.
 * @param state Pointer to the Geoscape.
 */
AlienBaseState::AlienBaseState(Game *game, AlienBase *base, GeoscapeState *state) : State(game), _state(state), _base(base)
{
	std::string background, backpalette;
	Uint8 colors[2];

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(50, 12, 135, 180);
	_txtTitle = new Text(320, 60, 0, 64);

	add(_window);
	add(_btnOk);
	add(_txtTitle);

	centerAllSurfaces();

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_BACK13.SCR";
		backpalette = "TFTD_BACKPALS.DAT";

		colors[0] = Palette::blockOffset(4);
		colors[1] = colors[2] = Palette::blockOffset(0)+1;
	}
	else
	{
		// Basic properties for display in UFO style
		background = "BACK13.SCR";
		backpalette = "BACKPALS.DAT";

		colors[0] = Palette::blockOffset(0);
		colors[1] = Palette::blockOffset(15)-1;
		colors[2] = Palette::blockOffset(8)+5;
	}

	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);

	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));
	
	_btnOk->setColor(colors[1]);
	_btnOk->setText(_game->getLanguage()->getString("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&AlienBaseState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&AlienBaseState::btnOkClick, (SDLKey)Options::getInt("keyOk"));
	_btnOk->onKeyboardPress((ActionHandler)&AlienBaseState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));

	_txtTitle->setColor(colors[2]);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);
	bool set(false);
	for (std::vector<Region*>::iterator k = _game->getSavedGame()->getRegions()->begin(); k != _game->getSavedGame()->getRegions()->end(); ++k)
	{
		if((*k)->getRules()->insideRegion(_base->getLongitude(), _base->getLatitude())) 
		{
			std::wstringstream ss;
			ss << _game->getLanguage()->getString("STR_XCOM_AGENTS_HAVE_LOCATED_AN_ALIEN_BASE_IN")
			    << _game->getLanguage()->getString((*k)->getRules()->getType());
			_txtTitle->setText(ss.str());
			set = true;
		}
	}
	if(!set)
	{
		std::wstringstream ss;
		ss << _game->getLanguage()->getString("STR_XCOM_AGENTS_HAVE_LOCATED_AN_ALIEN_BASE_IN")
		    << _game->getLanguage()->getString("STR_UNKNOWN");
		_txtTitle->setText(ss.str());
	}
}

/**
 *
 */
AlienBaseState::~AlienBaseState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void AlienBaseState::btnOkClick(Action *)
{
	_state->timerReset();
	_state->getGlobe()->center(_base->getLongitude(), _base->getLatitude());
	_game->popState();
}

}
