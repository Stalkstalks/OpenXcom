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
#include "BaseDefenseState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Ruleset/Ruleset.h"
#include "../Ruleset/RuleBaseFacility.h"
#include "../Savegame/Ufo.h"
#include "../Interface/TextList.h"
#include "GeoscapeState.h"
#include "../Engine/Action.h"
#include "../Engine/RNG.h"
#include "../Battlescape/BriefingState.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Engine/Sound.h"
#include "BaseDestroyedState.h"
#include "../Engine/Timer.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Base Defense screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base being attacked.
 * @param ufo Pointer to the attacking ufo.
 * @param state Pointer to the Geoscape.
 */
BaseDefenseState::BaseDefenseState(Game *game, Base *base, Ufo *ufo, GeoscapeState *state) : State(game), _state(state)
{
	std::string background, palette, backpalette;
	Uint8 colors[3];

	_base = base;
	_action = BDA_NONE;
	_row = -1;
	_passes = 0;
	_attacks = 0;
	_thinkcycles = 0;
	_ufo = ufo;
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(300, 16, 16, 6);
	_txtInit = new Text(300, 10, 16, 24);
	_lstDefenses = new TextList(300, 130, 16, 40);
	_btnOk = new TextButton(120, 18, 100, 170);

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		background = "TFTD_BACK04.SCR";
		palette = "TFTD_PALETTES.DAT_1";
		backpalette = "TFTD_BACKPALS.DAT";

		colors[0] = Palette::blockOffset(3);
		colors[1] = colors[2] = Palette::blockOffset(0)+1;
	}
	else
	{
		// Basic properties for display in UFO style
		background = "BACK04.SCR";
		palette = "PALETTES.DAT_1";
		backpalette = "PALETTES.DAT_1";

		colors[0] = Palette::blockOffset(14);
		colors[1] = Palette::blockOffset(15)+6;
		colors[2] = Palette::blockOffset(13)+10;
	}

	// Set palette
	_game->setPalette(_game->getResourcePack()->getPalette(palette)->getColors());
	_game->getResourcePack()->getSurface(background)->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(colors[0]), Palette::backPos, 16);

	add(_window);
	add(_btnOk);
	add(_txtTitle);
	add(_txtInit);
	add(_lstDefenses);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(colors[1]);
	_window->setBackground(_game->getResourcePack()->getSurface(background));

	_btnOk->setColor(colors[2]);
	_btnOk->setText(_game->getLanguage()->getString("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&BaseDefenseState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&BaseDefenseState::btnOkClick, (SDLKey)Options::getInt("keyOk"));
	_btnOk->onKeyboardPress((ActionHandler)&BaseDefenseState::btnOkClick, (SDLKey)Options::getInt("keyCancel"));
	_btnOk->setVisible(false);

	_txtTitle->setColor(colors[2]);
	_txtTitle->setBig();
	std::wstringstream ss;
	ss << _base->getName() << _game->getLanguage()->getString("STR_UNDER_ATTACK");
	_txtTitle->setText(ss.str());
	_txtInit->setVisible(false);

	_txtInit->setColor(colors[2]);
	_txtInit->setText(_game->getLanguage()->getString("STR_BASE_DEFENSES_INITIATED"));

	_lstDefenses->setColor(colors[2]);
	_lstDefenses->setColumns(3, 134, 70, 50);
	_gravShields = _base->getGravShields();
	_defenses = _base->getDefenses()->size();
	_timer = new Timer(750);
	_timer->onTimer((StateHandler)&BaseDefenseState::nextStep);
	_timer->start();
}
/**
 *
 */
BaseDefenseState::~BaseDefenseState()
{
	delete _timer;
}

/**
 * Resets the palette.
 */
void BaseDefenseState::init()
{
	std::string palette, backpalette;
	Uint8 color;

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		palette = "TFTD_PALETTES.DAT_1";
		backpalette = "TFTD_BACKPALS.DAT";

		color = Palette::blockOffset(3);
	}
	else
	{
		// Basic properties for display in UFO style
		palette = "PALETTES.DAT_1";
		backpalette = "PALETTES.DAT_1";

		color = Palette::blockOffset(14);
	}

	_game->setPalette(_game->getResourcePack()->getPalette(palette)->getColors());
	_game->setPalette(_game->getResourcePack()->getPalette(backpalette)->getColors(color), Palette::backPos, 16);
}

void BaseDefenseState::think()
{
	_timer->think(this, 0);
}

void BaseDefenseState::nextStep()
{
	if (_thinkcycles == -1)
		return;
	
	++_thinkcycles;

	if (_thinkcycles == 1)
	{
		_txtInit->setVisible(true);
		return;
	}

	if (_thinkcycles > 1)
	{
		switch (_action)
		{
		case BDA_DESTROY:
			_lstDefenses->addRow(2, _game->getLanguage()->getString("STR_UFO_DESTROYED").c_str(),L" ",L" ");
			_game->getResourcePack()->getSound("GEO.CAT", 11)->play();
			_action = BDA_END;
			return;
		case BDA_END:
			_btnOk->setVisible(true);
			_thinkcycles = -1;
			return;
		default:
			break;
		}
		if (_attacks == _defenses && _passes == _gravShields)
		{
			_action = BDA_END;
			return;
		}
		else if (_attacks == _defenses && _passes < _gravShields)
		{
			_lstDefenses->addRow(3, _game->getLanguage()->getString("STR_GRAV_SHIELD_REPELS_UFO").c_str(),L" ",L" ");
			++_row;
			++_passes;
			_attacks = 0;
			return;
		}
		
	

		BaseFacility* def = _base->getDefenses()->at(_attacks);
		
		switch (_action)
		{
		case  BDA_NONE:
			_lstDefenses->addRow(3, _game->getLanguage()->getString((def)->getRules()->getType()).c_str(),L" ",L" ");
			++_row;
			_action = BDA_FIRE;
			return;
		case BDA_FIRE:
			_lstDefenses->setCellText(_row, 1, _game->getLanguage()->getString("STR_FIRING").c_str());
			_game->getResourcePack()->getSound("GEO.CAT", (def)->getRules()->getFireSound())->play();
			_action = BDA_RESOLVE;
			return;
		case BDA_RESOLVE:
			if (RNG::generate(0, 100) > (def)->getRules()->getHitRatio())
			{
				_lstDefenses->setCellText(_row, 2, _game->getLanguage()->getString("STR_MISSED").c_str());
			}
			else
			{
				_lstDefenses->setCellText(_row, 2, _game->getLanguage()->getString("STR_HIT").c_str());
				_game->getResourcePack()->getSound("GEO.CAT", (def)->getRules()->getHitSound())->play();
				_ufo->setDamage(_ufo->getDamage() + (def)->getRules()->getDefenseValue());
			}
			if (_ufo->getStatus() == Ufo::DESTROYED)
				_action = BDA_DESTROY;
			else
				_action = BDA_NONE;
			++_attacks;
			return;
		default:
			break;
		}
	}
}
/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BaseDefenseState::btnOkClick(Action *)
{
	std::string palette;

	if (Options::getString("GUIstyle") == "xcom2")
	{
		// Basic properties for display in TFTD style
		palette = "TFTD_PALETTES.DAT_1";
	}
	else
	{
		// Basic properties for display in UFO style
		palette = "PALETTES.DAT_1";
	}

	_timer->stop();
	_game->setPalette(_game->getResourcePack()->getPalette(palette)->getColors());
	_game->popState();
	if(_ufo->getStatus() != Ufo::DESTROYED)
	{
		// Whatever happens in the base defense, the UFO has finished its duty
		_ufo->setStatus(Ufo::DESTROYED);
        _state->handleBaseDefense(_base, _ufo);
	}
}
}
