#include "services/ImageStore.h"

#include <cctype>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
using namespace std;

static bool is_safe_filename(const string& name) {
    if (name.empty()) {
        return false;
    }
    if (name.find('/') != string::npos ||
        name.find('\\') != string::npos ||
        name.find(':') != string::npos ||
        name.find("..") != string::npos) {
        return false;
    }
    return true;
}

static string lowercase_string(string value) {
    for (char& character : value) {
        character = static_cast<char>(
            tolower(static_cast<unsigned char>(character)));
    }
    return value;
}

static bool is_supported_image_extension(const string& ext) {
    const string lower_ext = lowercase_string(ext);
    return lower_ext == ".png" || lower_ext == ".jpg" ||
           lower_ext == ".jpeg" || lower_ext == ".bmp" ||
           lower_ext == ".tga" || lower_ext == ".gif";
}

ImageStore::ImageStore(const string& storage_dir)
    : textures_(), storage_dir_(storage_dir) {
    if (storage_dir_.empty()) {
        storage_dir_ = "resources/images/posts/";
    }

    for (char& character : storage_dir_) {
        if (character == '\\') {
            character = '/';
        }
    }

    if (storage_dir_.back() != '/') {
        storage_dir_ += '/';
    }

    error_code error;
    filesystem::create_directories(storage_dir_, error);
    if (error) {
        throw runtime_error("Could not create image storage directory");
    }
}

ImageStore::~ImageStore() {
    clear();
}

const Texture2D* ImageStore::get(const string& name) {
    if (!is_safe_filename(name)) {
        return nullptr;
    }

    auto cached = textures_.find(name);
    if (cached != textures_.end()) {
        ++cached->second.reference_count;
        return &cached->second.texture;
    }

    const string full_path = storage_dir_ + name;
    error_code error;
    if (!filesystem::exists(full_path, error) || error) {
        return nullptr;
    }

    Texture2D loaded_texture = LoadTexture(full_path.c_str());
    if (loaded_texture.id == 0) {
        return nullptr;
    }

    TextureEntry entry{};
    entry.texture = loaded_texture;
    entry.reference_count = 1;

    try {
        const auto inserted = textures_.emplace(name, entry);
        if (!inserted.second) {
            UnloadTexture(loaded_texture);
            ++inserted.first->second.reference_count;
            return &inserted.first->second.texture;
        }
        return &inserted.first->second.texture;
    } catch (...) {
        UnloadTexture(loaded_texture);
        throw;
    }
}

string ImageStore::import_file(const string& source_path, int post_id) {
    if (post_id <= 0 || source_path.empty()) {
        throw invalid_argument("Image source or post ID is invalid");
    }

    const filesystem::path source(source_path);
    error_code error;
    if (!filesystem::exists(source, error) || error) {
        throw invalid_argument("Image source file does not exist");
    }

    error.clear();
    if (!filesystem::is_regular_file(source, error) || error) {
        throw invalid_argument("Image source is not a regular file");
    }

    const string extension =
        lowercase_string(source.extension().string());
    if (!is_supported_image_extension(extension)) {
        throw invalid_argument("Image file extension is not supported");
    }

    error.clear();
    filesystem::create_directories(storage_dir_, error);
    if (error) {
        throw runtime_error("Could not create image storage directory");
    }

    size_t counter = 1;
    string dest_name;
    string destination_path;
    while (true) {
        dest_name = "post_" + to_string(post_id) + "_" +
                    to_string(counter) + extension;
        destination_path = storage_dir_ + dest_name;

        error.clear();
        const bool destination_exists =
            filesystem::exists(destination_path, error);
        if (error) {
            throw runtime_error("Could not check image destination");
        }
        if (!destination_exists) {
            break;
        }
        if (counter == (numeric_limits<size_t>::max)()) {
            throw runtime_error("No image destination name is available");
        }
        ++counter;
    }

    error.clear();
    filesystem::copy_file(source, destination_path,
                               filesystem::copy_options::none, error);
    if (error) {
        throw runtime_error("Could not copy image into storage directory");
    }

    return dest_name;
}

void ImageStore::release(const string& name) {
    const auto found = textures_.find(name);
    if (found != textures_.end() && found->second.reference_count > 0) {
        --found->second.reference_count;
    }
}

void ImageStore::remove_if_unused(const string& name) {
    const auto found = textures_.find(name);
    if (found == textures_.end() || found->second.reference_count != 0) {
        return;
    }

    if (found->second.texture.id != 0) {
        UnloadTexture(found->second.texture);
    }
    textures_.erase(found);
}

void ImageStore::delete_file(const string& name) {
    if (!is_safe_filename(name)) {
        return;
    }

    const auto cached = textures_.find(name);
    if (cached != textures_.end()) {
        if (cached->second.texture.id != 0) {
            UnloadTexture(cached->second.texture);
        }
        textures_.erase(cached);
    }

    const string full_path = storage_dir_ + name;
    error_code error;
    if (filesystem::exists(full_path, error) && !error) {
        filesystem::remove(full_path, error);
        if (error) {
            throw runtime_error("Could not remove image file");
        }
    } else if (error) {
        throw runtime_error("Could not check image file before removal");
    }
}

void ImageStore::clear() {
    for (auto current = textures_.begin(); current != textures_.end(); ++current) {
        if (current->second.texture.id != 0) {
            UnloadTexture(current->second.texture);
        }
    }
    textures_.clear();
}
