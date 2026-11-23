#include "services/PostFactory.h"
#include <cstdint>
#include <istream>
#include <memory>
#include <stdexcept>

Post* PostFactory::create(PostType type) {
    switch (type) {
    case PostType::Photo:
    case PostType::Text:
    case PostType::Carousel:
    case PostType::Poll:
    case PostType::Notice:
    case PostType::LostFound:
        // Concrete Photo, Text, Carousel, Poll, Notice, and LostFound post classes are not present yet.
        throw std::runtime_error("Post type is valid but has no concrete implementation");
    default: throw std::invalid_argument("Invalid post type");
    }
}
Post* PostFactory::read_from_stream(std::istream& in) {
    std::uint8_t raw = 0; in.read(reinterpret_cast<char*>(&raw), sizeof(raw));
    if (!in) throw std::runtime_error("Failed to read post type");
    if (raw < static_cast<std::uint8_t>(PostType::Photo) || raw > static_cast<std::uint8_t>(PostType::LostFound)) throw std::invalid_argument("Invalid post type tag");
    std::unique_ptr<Post> post(create(static_cast<PostType>(raw)));
    post->read(in);
    return post.release();
}
