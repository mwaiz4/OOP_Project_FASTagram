#ifndef FASTAGRAM_MODELS_TEXT_POST_H
#define FASTAGRAM_MODELS_TEXT_POST_H

#include "models/Post.h"

#include <string>

class TextPost : public Post {
public:
    TextPost();
    TextPost(int author_id, const Time& created, const std::string& caption,
             const std::string& body_text, int bg_style = 0);

    PostType type() const noexcept override { return PostType::Text; }
    std::string type_name() const;
    std::string search_text() const override;
    float render_height(float card_width) const;

    // Post.h currently uses a Rectangle for its virtual draw interface.
    void draw(DrawContext& context, const Rectangle& bounds) const override;
    // This overload is kept for the requested coordinate-based card interface.
    void draw(DrawContext& context, float x, float y, float width) const;
    bool handle_click(float mouse_x, float mouse_y,
                      float x, float y, float width, int viewer_id);

    const std::string& body_text() const noexcept;
    int bg_style() const noexcept;
    void set_body_text(const std::string& body_text);
    void set_bg_style(int bg_style);

    void write(std::ostream& out) const override;
    void read(std::istream& in) override;

private:
    std::string body_text_;
    int bg_style_;
};

#endif // FASTAGRAM_MODELS_TEXT_POST_H
