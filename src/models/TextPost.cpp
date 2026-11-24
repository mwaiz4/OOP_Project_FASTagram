#include "models/TextPost.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
using namespace std;

TextPost::TextPost()
    : Post(), body_text_(), bg_style_(0) {}

TextPost::TextPost(int author_id, const Time& created, const string& caption,
                   const string& body_text, int bg_style)
    : Post(author_id, created, caption),
      body_text_(body_text),
      bg_style_(bg_style) {
    if (body_text_.empty()) {
        throw invalid_argument("Text post body cannot be empty");
    }
    if (bg_style_ < 0) {
        throw invalid_argument("Text post background style cannot be negative");
    }
}

string TextPost::type_name() const {
    return "Text";
}

string TextPost::search_text() const {
    return caption() + " " + body_text_;
}

float TextPost::render_height(float card_width) const {
    if (!isfinite(card_width) || card_width <= 0.0f) {
        return 0.0f;
    }

    const double characters_per_line =
        (max)(1.0, static_cast<double>(card_width) / 8.0);
    const double line_count = ceil(
        static_cast<double>(body_text_.size()) / characters_per_line);
    const float caption_height = caption().empty() ? 0.0f : 22.0f;
    return 24.0f + static_cast<float>(line_count * 20.0) + caption_height;
}

void TextPost::draw(DrawContext& context, const Rectangle& bounds) const {
    (void)context;
    (void)bounds;
}

void TextPost::draw(DrawContext& context, float x, float y, float width) const {
    (void)context;
    (void)x;
    (void)y;
    (void)width;
}

bool TextPost::handle_click(float mouse_x, float mouse_y,
                            float x, float y, float width, int viewer_id) {
    (void)mouse_x;
    (void)mouse_y;
    (void)x;
    (void)y;
    (void)width;
    (void)viewer_id;
    return false;
}

const string& TextPost::body_text() const noexcept {
    return body_text_;
}

int TextPost::bg_style() const noexcept {
    return bg_style_;
}

void TextPost::set_body_text(const string& body_text) {
    if (body_text.empty()) {
        throw invalid_argument("Text post body cannot be empty");
    }
    body_text_ = body_text;
}

void TextPost::set_bg_style(int bg_style) {
    if (bg_style < 0) {
        throw invalid_argument("Text post background style cannot be negative");
    }
    bg_style_ = bg_style;
}

void TextPost::write(ostream& out) const {
    if (body_text_.empty()) {
        throw runtime_error("Text post body cannot be empty when saving");
    }
    if (body_text_.size() > 1000000) {
        throw runtime_error("Text post body is too long to save");
    }
    if (bg_style_ < 0 || bg_style_ > (numeric_limits<int32_t>::max)()) {
        throw runtime_error("Text post background style is invalid");
    }

    Post::write(out);

    const uint32_t body_length = static_cast<uint32_t>(body_text_.size());
    out.write(reinterpret_cast<const char*>(&body_length), sizeof(body_length));
    out.write(body_text_.data(), static_cast<streamsize>(body_text_.size()));

    const int32_t saved_bg_style = static_cast<int32_t>(bg_style_);
    out.write(reinterpret_cast<const char*>(&saved_bg_style), sizeof(saved_bg_style));
    if (!out) {
        throw runtime_error("Failed to write text post fields");
    }
}

void TextPost::read(istream& in) {
    Post::read(in);

    uint32_t body_length = 0;
    in.read(reinterpret_cast<char*>(&body_length), sizeof(body_length));
    if (!in || body_length == 0 || body_length > 1000000) {
        throw runtime_error("File contains an invalid text post body length");
    }

    string loaded_body(body_length, '\0');
    in.read(&loaded_body[0], static_cast<streamsize>(body_length));
    if (!in) {
        throw runtime_error("Failed to read text post body");
    }

    int32_t loaded_bg_style = 0;
    in.read(reinterpret_cast<char*>(&loaded_bg_style), sizeof(loaded_bg_style));
    if (!in || loaded_bg_style < 0) {
        throw runtime_error("File contains an invalid text post background style");
    }

    body_text_.swap(loaded_body);
    bg_style_ = static_cast<int>(loaded_bg_style);
}
