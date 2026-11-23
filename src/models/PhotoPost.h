#ifndef FASTAGRAM_MODELS_PHOTO_POST_H
#define FASTAGRAM_MODELS_PHOTO_POST_H

#include "models/Post.h"

#include <string>

class PhotoPost : public Post {
public:
    PhotoPost();
    PhotoPost(int author_id, const Time& created,
              const std::string& caption, const std::string& image_path);

    PostType type() const noexcept override { return PostType::Photo; }
    std::string type_name() const;
    std::string search_text() const override;
    float render_height(float card_width) const;

    // Post.h currently uses a Rectangle for its virtual draw interface.
    void draw(DrawContext& context, const Rectangle& bounds) const override;
    // This overload is kept for the requested coordinate-based card interface.
    void draw(DrawContext& context, float x, float y, float width) const;
    bool handle_click(float mouse_x, float mouse_y,
                      float x, float y, float width, int viewer_id);

    const std::string& image_path() const noexcept;
    void set_image_path(const std::string& path);

    void write(std::ostream& out) const override;
    void read(std::istream& in) override;

private:
    std::string image_path_;
};

#endif // FASTAGRAM_MODELS_PHOTO_POST_H
