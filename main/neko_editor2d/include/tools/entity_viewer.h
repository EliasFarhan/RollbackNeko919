#pragma once

/*
 MIT License

 Copyright (c) 2019 SAE Institute Switzerland AG

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
#include <string>
#include <vector>

#include <engine/entity.h>
#include <engine/transform.h>
#include "editor_scene.h"

namespace neko::editor
{
class EditorPrefabManager;
class NekoEditor;
enum class EditorMode : std::uint8_t;
class EntityViewer
{
public:
    EntityViewer(NekoEditorExport& editorExport);
    void Update(EditorMode editorMode);

    void Reset()
    { selectedEntity_ = neko::INVALID_ENTITY; }

    neko::Entity GetSelectedEntity()
    { return selectedEntity_; }

private:
    void DrawEntityHierarchy(neko::Entity entity, size_t index, std::set<neko::Entity>& entitySet, bool draw,
                             bool destroy);

    std::vector<Entity> entities_;
    std::vector<std::string> entitiesName_;
    Entity selectedEntity_ = neko::INVALID_ENTITY;
    sfml::Transform2dManager& transformManager_;
    EntityManager& entityManager_;
    EditorSceneManager& sceneManager_;
    EditorPrefabManager& prefabManager_;
    NekoEditor& nekoEditor_;
};
}