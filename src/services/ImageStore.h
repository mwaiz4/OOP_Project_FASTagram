#pragma once

#include "raylib.h"

#include <cstddef>
#include <map>
#include <string>

class ImageStore {
public:
    explicit ImageStore(
        const std::string& storage_dir = "resources/images/posts/");
    ~ImageStore();

    ImageStore(const ImageStore&) = delete;
    ImageStore& operator=(const ImageStore&) = delete;

    // The returned pointer is owned by ImageStore and stays valid only while
    // its cached entry remains loaded. Call release() when finished with it.
    const Texture2D* get(const std::string& name);
    std::string import_file(const std::string& source_path, int post_id);
    void release(const std::string& name);
    void remove_if_unused(const std::string& name);
    void delete_file(const std::string& name);
    void clear();

private:
    struct TextureEntry {
        Texture2D texture;
        std::size_t reference_count;
    };

    // ImageStore owns every texture stored in this cache.
    std::map<std::string, TextureEntry> textures_;
    std::string storage_dir_;
};
