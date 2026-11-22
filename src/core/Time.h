#pragma once

#include <iosfwd>
#include <string>

class Time {
public:
    using seconds_type = long long;
    using clock_fn = seconds_type (*)();

    // Stores seconds since the Unix epoch. Negative values are invalid.
    explicit Time(seconds_type seconds = 0);

    static Time now();

    // Pass nullptr to use the normal system clock again.
    static void set_clock(clock_fn fn) noexcept;

    seconds_type seconds() const noexcept;
    std::string label() const;

    void write(std::ostream& out) const;
    void read(std::istream& in);

    friend seconds_type operator-(const Time& lhs,
                                  const Time& rhs) noexcept;
    friend bool operator<(const Time& lhs, const Time& rhs) noexcept;
    friend bool operator>(const Time& lhs, const Time& rhs) noexcept;
    friend bool operator==(const Time& lhs, const Time& rhs) noexcept;
    friend bool operator!=(const Time& lhs, const Time& rhs) noexcept;
    friend std::ostream& operator<<(std::ostream& out, const Time& time);

private:
    seconds_type seconds_;
    static clock_fn clock_source_;
};
