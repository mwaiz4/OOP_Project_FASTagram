#include "services/PostFactory.h"

#include "models/PhotoPost.h"
#include "models/TextPost.h"

#include <cstdint>
#include <istream>
#include <memory>
#include <stdexcept>

Post* PostFactory::create(PostType type) {
    switch (type) {
    case PostType::Photo:
        return new PhotoPost();
    case PostType::Text:
        return new TextPost();
    case PostType::Carousel:
    case PostType::Poll:
    case PostType::Notice:
    case PostType::LostFound:
        throw std::runtime_error("This post type is not implemented yet");
    default:
        throw std::invalid_argument("Invalid post type");
    }
}

Post* PostFactory::read_from_stream(std::istream& in) {
    std::uint8_t raw_type = 0;
    in.read(reinterpret_cast<char*>(&raw_type), sizeof(raw_type));
    if (!in) {
        throw std::runtime_error("Failed to read post type tag");
    }

    if (raw_type < static_cast<std::uint8_t>(PostType::Photo) ||
        raw_type > static_cast<std::uint8_t>(PostType::LostFound)) {
        throw std::invalid_argument("Invalid post type tag");
    }

    std::unique_ptr<Post> post(create(static_cast<PostType>(raw_type)));
    if (post == nullptr) {
        throw std::runtime_error("Post factory returned no post");
    }
    post->read(in);
    return post.release();
}
