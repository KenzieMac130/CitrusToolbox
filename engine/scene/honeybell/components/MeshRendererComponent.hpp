/*
   Copyright 2021 MacKenzie Strand

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "utilities/Common.h"

#include "../Component.hpp"

namespace ctHoneybell {

class CT_API MeshRendererComponent : public ComponentBase {
public:
   MeshRendererComponent(struct ConstructContext ctx,
                         class ToyBase* _toy,
                         int32_t meshInstanceIdx = 0);
   ~MeshRendererComponent();

   virtual ctResults Begin(struct BeginContext& ctx);

   virtual bool hasTransform() const;
   virtual void SetWorldTransform(ctTransform& v);
   virtual ctTransform GetWorldTransform() const;

   virtual void SetLocalBounds(ctBoundBox& v);
   virtual ctBoundBox GetLocalBounds() const;

   virtual const char* GetTypeName();

private:
   ctTransform transform;
   ctBoundBox bounds;
};
}