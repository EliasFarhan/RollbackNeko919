#pragma once

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
#include <vector>
#include <memory>
#include <spine/spine-sfml.h>
#include "engine/entity.h"
#include "engine/component.h"
#include "sfml_engine/graphics.h"


namespace neko
{

class Scale2dManager;

namespace sfml
{
class SfmlGraphicsManager;

class SfmlPosition2dManager;

class SfmlRotation2dManager;

class SfmlTransform2dManager;

struct BasicSpineDrawable : public SfmlRenderCommand
{
    BasicSpineDrawable();

    virtual ~BasicSpineDrawable();

    Atlas* atlas = nullptr;
    SkeletonData* skeletonData = nullptr;
    std::shared_ptr<spine::SkeletonDrawable> skeletonDrawable = nullptr;
    int layer = 0;
    sf::Transform transform{};

    void SetAnimationByName(std::string_view animName);

    void SetSkinByName(std::string_view skinName);

    void SetPosition(const sf::Vector2f& position);

    sf::Vector2f GetPosition();
};

struct SpineBoneFollower
{
    Entity followingEntity = INVALID_ENTITY;
    Bone* followingBone = nullptr;
};

struct SpineDrawableInfo
{
    std::string spinePath = "";
    std::string atlasPath = "";
    std::string skeletonDataPath = "";
};

class SpineBoneFollowerManager : public ComponentManager<SpineBoneFollower, EntityMask(
        NekoComponentType::SPINE_FOLLOW_BONE)>
{

};

class SpineManager : public ComponentManager<BasicSpineDrawable, EntityMask(NekoComponentType::SPINE_ANIMATION)>
{
public:
    SpineManager();

    void Update(EntityManager& entityManager, float dt);

    bool AddSpineDrawable(Entity entity,
                          const std::string_view atlasFilename,
                          const std::string_view skeletonFilename);

    void ParseComponentJson(json& componentJson, Entity entity) override;

    void CopyAllTransforms(EntityManager& entityManager, SfmlTransform2dManager& transformManager);

    void CopyAllTransformPositions(EntityManager& entityManager, SfmlPosition2dManager& position2Manager);

    void CopyAllTransformScales(EntityManager& entityManager, Scale2dManager& scale2DManager);

    void CopyAllTransformAngles(EntityManager& entityManager, SfmlRotation2dManager& angle2DManager);

    void PushAllCommands(EntityManager& entityManager, SfmlGraphicsManager& graphicsManager);

    void CopyLayer(int layer, size_t start, size_t length);

    Index AddComponent(EntityManager& entityManager, Entity entity) override;

    json SerializeComponentJson(Entity entity) override;

    SpineDrawableInfo& GetInfo(Entity entity);

    void DestroyComponent(EntityManager& entityManager, Entity entity) override;

private:
    SpineBoneFollowerManager spineBoneFollowerManager_;
    std::vector<SpineDrawableInfo> infos_;
};


SkeletonData* readSkeletonJsonData(const char* filename, Atlas* atlas, float scale);

SkeletonData* readSkeletonBinaryData(const char* filename, Atlas* atlas, float scale);
}
}