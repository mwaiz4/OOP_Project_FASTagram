#pragma once
#include "models/Post.h"
#include <iosfwd>
class PostFactory {
public:
    static Post* create(PostType type);
    static Post* read_from_stream(std::istream& in);
};
