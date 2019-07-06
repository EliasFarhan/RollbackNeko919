/*
 MIT License

 Copyright (c) 2017 SAE Institute Switzerland AG

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <City/city_command.h>
#include <City/city_zone.h>
#include "engine/globals.h"
#include "engine/engine.h"
#include "City/city_engine.h"
#include "City/city_editor.h"

namespace neko
{
void CityCommandManager::AddCommand(std::unique_ptr<CityCommand> command, bool fromRenderThread)
{
	const Index frameIndex = (MainEngine::GetInstance()->frameIndex - (fromRenderThread ? 1 : 0)) % 2;
	if (command->commandType == CityCommandType::CHANGE_CURSOR_MODE)
	{
		commandQueue_[frameIndex].insert(commandQueue_[frameIndex].begin(), std::move(command));
		for(auto i = 0u; i < commandQueue_[frameIndex].size();i++)
		{
			if(commandQueue_[frameIndex][i]->commandType == CityCommandType::CREATE_CITY_ELEMENT)
			{
				commandQueue_[frameIndex].erase(commandQueue_[frameIndex].begin() + i);
				i--;
			}
		}
	}
	else
	{
		commandQueue_[(frameIndex+1)%2].push_back(std::move(command));
	}

}

void CityCommandManager::Init()
{
	engine_ = dynamic_cast<CityBuilderEngine*>(MainEngine::GetInstance());
}

void CityCommandManager::ExecuteCommand(const std::shared_ptr<CityCommand>& command) const
{
	switch (command->commandType)
	{
	case CityCommandType::CHANGE_CURSOR_MODE:
	{
		auto* cursorCommand = dynamic_cast<ChangeModeCommand*>(command.get());
		engine_->GetCursor().SetCursorMode(cursorCommand->newCursorMode);
		break;
	}
	case CityCommandType::CREATE_CITY_ELEMENT:
	{
		auto* buildCommand = dynamic_cast<BuildElementCommand*>(command.get());
		engine_->GetZoneManager().RemoveZone(buildCommand->position);
		engine_->GetCityMap().AddCityElement(buildCommand->elementType, buildCommand->position);
		if (buildCommand->elementType == CityElementType::ROAD)
		{
			engine_->GetBuildingManager().RemoveBuilding(buildCommand->position);
		}
		break;
	}
	case CityCommandType::DELETE_CITY_ELEMENT:
	{
		auto* buildCommand = dynamic_cast<DestroyElementCommand*>(command.get());
		engine_->GetCityMap().RemoveCityElement(buildCommand->position);
		engine_->GetCarManager().RescheduleCarPathfinding(buildCommand->position);
		engine_->GetZoneManager().RemoveZone(buildCommand->position);
		engine_->GetBuildingManager().RemoveBuilding(buildCommand->position);
		break;
	}
	case CityCommandType::ADD_CITY_ZONE:
	{
		auto* zoneCommand = dynamic_cast<AddZoneCommand*>(command.get());
		engine_->GetZoneManager().AddZone(zoneCommand->position, zoneCommand->zoneType, engine_->GetCityMap());
		break;
	}
	default:
		break;
	}
}

void CityCommandManager::Update(float dt)
{
	const Index frameIndex = (MainEngine::GetInstance()->frameIndex) % 2;
	const auto commandNmb = commandQueue_[frameIndex].size();
	for (auto i = 0u; i < commandNmb; i++)
	{
		auto command = commandQueue_[frameIndex][i];
		ExecuteCommand(command);
	}
	commandQueue_[frameIndex].clear();
}

void CityCommandManager::Destroy()
{
}
}
