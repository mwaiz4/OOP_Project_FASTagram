#pragma once
#include "models/Post.h"
#include <string>
#include <vector>
class CarouselPost : public Post {
public:
    CarouselPost();
    CarouselPost(int author_id, const Time& created, const std::string& caption, const std::vector<std::string>& image_paths);
    PostType type() const override { return PostType::Carousel; }
    std::string type_name() const;
    std::string search_text() const override;
    float render_height(float card_width) const;
    void draw(DrawContext& context, const Rectangle& bounds) const override;
    void draw(DrawContext& context, float x, float y, float width) const;
    bool handle_click(float mouse_x, float mouse_y, float x, float y, float width, int viewer_id);
    void add_image(const std::string& path);
    const std::vector<std::string>& image_paths() const;
    int image_count() const;
    int current_index() const;
    void next_slide();
    void prev_slide();
    void set_current_index(int index);
    std::string current_image_path() const;
    void write(std::ostream& out) const override;
    void read(std::istream& in) override;
private:
    std::vector<std::string> image_paths_;
    int current_index_;
};
