/*
 MIT License

 Copyright (c) 2020 SAE Institute Switzerland AG

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
#include <engine/conversion.h>
#include "asteroid/rollback_manager.h"
#include "asteroid/game_manager.h"

namespace neko::asteroid
{


RollbackManager::RollbackManager(GameManager& gameManager, EntityManager& entityManager):
	gameManager_(gameManager), currentTransformManager_(entityManager),
	currentPhysicsManager_(entityManager), lastValidatePhysicsManager_(entityManager),
	currentPlayerManager_(entityManager, currentPhysicsManager_),
	lastValidatePlayerCharacter_(entityManager, lastValidatePhysicsManager_)
{
	for(auto& input: inputs_)
	{
		std::fill(input.begin(), input.end(), 0u);
	}
}

void RollbackManager::SimulateToCurrentFrame()
{
	auto currentFrame = gameManager_.GetCurrentFrame();
	auto lastValidateFrame = gameManager_.GetLastValidateFrame();
	currentPhysicsManager_ = lastValidatePhysicsManager_;
	currentPlayerManager_ = lastValidatePlayerCharacter_;
	for(net::Frame frame = lastValidateFrame+1; frame <= currentFrame; frame++)
    {
	    //Copy player input to player manager
        for(net::PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            const auto playerInput = GetInputAtFrame(playerNumber, frame);
            const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
            auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
            playerCharacter.input = playerInput;
            currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
        }
        currentPlayerManager_.FixedUpdate(seconds(GameManager::FixedPeriod));
	    currentPhysicsManager_.FixedUpdate(seconds(GameManager::FixedPeriod));
    }
    for(net::PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
        const auto playerBody = currentPhysicsManager_.GetBody(playerEntity);
        currentTransformManager_.SetPosition(playerEntity, playerBody.position);
        currentTransformManager_.SetRotation(playerEntity, playerBody.rotation);
    }
}
void RollbackManager::SetPlayerInput(net::PlayerNumber playerNumber, net::PlayerInput playerInput, std::uint32_t inputFrame)
{
	//Should only be called on the server
	if(currentFrame_ < inputFrame)
	{
		StartNewFrame(inputFrame);
	}
	inputs_[playerNumber][currentFrame_ - inputFrame] = playerInput;
	if(lastReceivedFrame_[playerNumber] < inputFrame)
	{
		lastReceivedFrame_[playerNumber] = inputFrame;
		//Repeat the same inputs until currentFrame
		for(size_t i = 0; i < currentFrame_-inputFrame;i++)
		{
			inputs_[playerNumber][i] = playerInput;
		}
	}
}

void RollbackManager::StartNewFrame(net::Frame newFrame)
{
	const auto delta = newFrame - currentFrame_;
	if(delta == 0)
	{
		return;
	}
	for (auto& inputs : inputs_)
	{
		for(auto i = inputs.size()-1; i >= delta; i--)
		{
			inputs[i] = inputs[i-delta];
		}

		for (std::uint32_t i = 0; i < delta; i++)
		{
			inputs[i] = inputs[delta];
		}
	}
	currentFrame_ = newFrame;
}

void RollbackManager::ValidateFrame(net::Frame newValidateFrame)
{
	for(net::PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++ )
	{
		if(GetLastReceivedFrame(playerNumber) < newValidateFrame)
		{
			neko_assert(false, "We should not validate a frame if we did not receive all inputs!!!");
			return;
		}
	}

    for(net::Frame frame = lastValidateFrame_ + 1; frame <= newValidateFrame; frame++)
    {
        for(net::PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            const auto playerInput = GetInputAtFrame(playerNumber, frame);
            const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
            auto playerCharacter = lastValidatePlayerCharacter_.GetComponent(playerEntity);
            playerCharacter.input = playerInput;
            lastValidatePlayerCharacter_.SetComponent(playerEntity, playerCharacter);
        }
        lastValidatePlayerCharacter_.FixedUpdate(seconds(GameManager::FixedPeriod));
        lastValidatePhysicsManager_.FixedUpdate(seconds(GameManager::FixedPeriod));
    }
	lastValidateFrame_ = newValidateFrame;
}
void RollbackManager::ConfirmFrame(net::Frame newValidateFrame, const std::array<PhysicsState, maxPlayerNmb>& serverPhysicsState)
{
    ValidateFrame(newValidateFrame);
    for(net::PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        const PhysicsState lastPhysicsState = GetValidatePhysicsState(playerNumber);
        if(serverPhysicsState[playerNumber] != lastPhysicsState)
        {
            neko_assert(false, "Physics State are not equal");
        }
    }
}
PhysicsState RollbackManager::GetValidatePhysicsState(net::PlayerNumber playerNumber) const
{
	PhysicsState state = 0;
    const Entity playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
    const auto& playerCharacter = lastValidatePhysicsManager_.GetBody(playerEntity);
	auto pos = playerCharacter.position;
	const auto* posPtr = reinterpret_cast<const PhysicsState*>(&pos);
	//Adding position
	for(size_t i = 0; i < sizeof(Vec2f)/sizeof(PhysicsState); i++)
	{
		state += posPtr[i];
	}

	//Adding velocity
	auto velocity = playerCharacter.velocity;
	const auto* velocityPtr = reinterpret_cast<const PhysicsState*>(&velocity);
	for(size_t i = 0; i < sizeof(Vec2f)/sizeof(PhysicsState); i++)
    {
	    state += velocityPtr[i];
    }
    //Adding rotation
    auto angle = playerCharacter.rotation;
    const auto* anglePtr = reinterpret_cast<const PhysicsState*>(&angle);
    for (size_t i = 0; i < sizeof(degree_t) / sizeof(PhysicsState); i++)
    {
        state += anglePtr[i];
    }
    //Adding angular Velocity
    auto angularVelocity = playerCharacter.angularVelocity;
    const auto* angularVelPtr = reinterpret_cast<const PhysicsState*>(&angularVelocity);
    for (size_t i = 0; i < sizeof(degree_t) / sizeof(PhysicsState); i++)
    {
        state += angularVelPtr[i];
    }
	return state;
}

void RollbackManager::SpawnPlayer(net::PlayerNumber playerNumber, Entity entity, Vec2f position, degree_t rotation)
{
    Body playerBody;
    playerBody.position = position;
    playerBody.rotation = rotation;
    currentPhysicsManager_.AddBody(entity);
    currentPhysicsManager_.SetBody(entity, playerBody);
    lastValidatePhysicsManager_.AddBody(entity);
    lastValidatePhysicsManager_.SetBody(entity, playerBody);

    currentTransformManager_.AddComponent(entity);
    currentTransformManager_.SetPosition(entity, position);
    currentTransformManager_.SetRotation(entity, rotation);
}

net::PlayerInput RollbackManager::GetInputAtFrame(net::PlayerNumber playerNumber, net::Frame frame)
{
	return inputs_[playerNumber][currentFrame_ - frame];
}

}
