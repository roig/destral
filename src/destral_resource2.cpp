#include <destral/destral_resource2.h>

namespace ds {

    void cp::resource::register_component(registry* r) {
        r->component_register<cp::resource>(cp::resource::name);
    }

    void cp::resource_loader::register_component(registry* r) {
        r->component_register<cp::resource_loader>(cp::resource_loader::name);
    }

    entity resource_get(registry* r, const char* resource_key_filepath) {
        dscheck(r);
        dscheck(resource_key_filepath);
        const i32 res_key_hashed = fnv1a_32bit(resource_key_filepath);

        // Try to find the resource entity if it is loaded.
        {
            auto v = r->view_create({ cp::resource::name });
            while (v.valid()) {
                cp::resource* res = v.data<cp::resource>(0);
                if (res->hashed_key == res_key_hashed) {
                    return v.entity();
                }
                v.next();
            }
        }

        // If no entity found with that key, try to load it.
        {
            auto v = r->view_create({ cp::resource_loader::name });
            while (v.valid()) {
                cp::resource_loader* loader = v.data<cp::resource_loader>(0);
                if (loader->can_load_fn(r, resource_key_filepath)) {
                    entity loaded_entity = loader->load_fn(r, resource_key_filepath);
                    // Setup the resource component data
                    if (r->entity_valid(loaded_entity)) {
                        cp::resource* res_cp = r->component_try_get<cp::resource>(loaded_entity, cp::resource::name);
                        dsverifym(res_cp, "Resources must have the cp::resource component");
                        res_cp->hashed_key = res_key_hashed;
                        res_cp->is_key_valid = true;
                        res_cp->key = resource_key_filepath;
                    }
                    return loaded_entity;
                }
                v.next();
            }
        }

        // No loader found
        return entity_null;
    }
}