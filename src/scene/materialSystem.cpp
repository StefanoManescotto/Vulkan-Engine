//
// Created by stefano on 17/09/26.
//

#include "materialSystem.h"

namespace MaterialSystem {
    std::vector<uint32_t> loadMaterials(const tg3_model &model, const std::vector<uint32_t> &textureIds, std::vector<Material> &materials) {
        std::vector<uint32_t> materialIds(model.materials_count);
        for (int i = 0; i < model.materials_count; ++i) {
            const tg3_material *tg3mat = &model.materials[i];
            materials.push_back( Material {
                    .baseColor = glm::vec4(
                        tg3mat->pbr_metallic_roughness.base_color_factor[0],
                        tg3mat->pbr_metallic_roughness.base_color_factor[1],
                        tg3mat->pbr_metallic_roughness.base_color_factor[2],
                        tg3mat->pbr_metallic_roughness.base_color_factor[3]),
                    .textureIndex = tg3mat->pbr_metallic_roughness.base_color_texture.index != -1
                        ? textureIds[tg3mat->pbr_metallic_roughness.base_color_texture.index] - 1
                        : 0
                });
            materialIds[i] = materials.size();
        }
        return materialIds;
    }
}
