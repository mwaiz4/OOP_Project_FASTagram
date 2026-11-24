#include "models/PhotoPost.h"

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>
using namespace std;

PhotoPost::PhotoPost()
    : Post(), image_path_() {}

PhotoPost::PhotoPost(int author_id, const Time& created,
                     const string& caption, const string& image_path)
    : Post(author_id, created, caption), image_path_(image_path) {
    if (image_path_.empty()) {
        throw invalid_argument("Photo post image path cannot be empty");
    }
}

string PhotoPost::type_name() const {
    return "Photo";
}

string PhotoPost::search_text() const {
    return caption();
}

float PhotoPost::render_height(float card_width) const {
    if (card_width <= 0.0f) {
        return 0.0f;
    }

    const float caption_height = caption().empty() ? 0.0f : 42.0f;
    return card_width + 24.0f + caption_height;
}

void PhotoPost::draw(DrawContext& context, const Rectangle& bounds) const {
    (void)context;
    (void)bounds;
}

void PhotoPost::draw(DrawContext& context, float x, float y, float width) const {
    (void)context;
    (void)x;
    (void)y;
    (void)width;
}

bool PhotoPost::handle_click(float mouse_x, float mouse_y,
                             float x, float y, float width, int viewer_id) {
    (void)mouse_x;
    (void)mouse_y;
    (void)x;
    (void)y;
    (void)width;
    (void)viewer_id;
    return false;
}

const string& PhotoPost::image_path() const noexcept {
    return image_path_;
}

void PhotoPost::set_image_path(const string& path) {
    if (path.empty()) {
        throw invalid_argument("Photo post image path cannot be empty");
    }
    image_path_ = path;
}

void PhotoPost::write(ostream& out) const {
    if (image_path_.size() > 1000000) {
        throw runtime_error("Photo path is too long to save");
    }

    Post::write(out);

    const uint32_t path_length = static_cast<uint32_t>(image_path_.size());
    out.write(reinterpret_cast<const char*>(&path_length), sizeof(path_length));
    out.write(image_path_.data(), static_cast<streamsize>(image_path_.size()));
    if (!out) {
        throw runtime_error("Failed to write photo path");
    }
}

void PhotoPost::read(istream& in) {
    Post::read(in);

    uint32_t path_length = 0;
    in.read(reinterpret_cast<char*>(&path_length), sizeof(path_length));
    if (!in || path_length > 1000000) {
        throw runtime_error("File contains an invalid photo path length");
    }

    string loaded_path(path_length, '\0');
    if (path_length > 0) {
        in.read(&loaded_path[0], static_cast<streamsize>(path_length));
    }
    if (!in) {
        throw runtime_error("Failed to read photo path");
    }

    image_path_.swap(loaded_path);
}
