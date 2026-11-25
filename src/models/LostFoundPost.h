#pragma once
#include "models/Post.h"
#include <string>
class LostFoundPost : public Post {
public:
    LostFoundPost();
    LostFoundPost(int author_id, const Time& created, const std::string& caption, bool is_lost, const std::string& item_name, const std::string& location, const std::string& contact_info, const std::string& image_path = "");
    PostType type() const override { return PostType::LostFound; }
    std::string type_name() const;
    std::string search_text() const override;
    float render_height(float card_width) const;
    void draw(DrawContext& context, const Rectangle& bounds) const override;
    void draw(DrawContext& context, float x, float y, float width) const;
    bool handle_click(float mouse_x, float mouse_y, float x, float y, float width, int viewer_id);
    bool is_lost() const; void set_is_lost(bool value);
    const std::string& item_name() const; void set_item_name(const std::string& value);
    const std::string& location() const; void set_location(const std::string& value);
    const std::string& contact_info() const; void set_contact_info(const std::string& value);
    const std::string& image_path() const; void set_image_path(const std::string& value);
    bool is_resolved() const; void set_resolved(bool value);
    void write(std::ostream& out) const override;
    void read(std::istream& in) override;
private:
    bool is_lost_;
    std::string item_name_, location_, contact_info_, image_path_;
    bool is_resolved_;
};
