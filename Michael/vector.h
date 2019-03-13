#include <iostream>

class Vector {
    int x;
    int y;
    int z;
public:
    Vector(): x(0), y(0), z(0) {}
    Vector(int x, int y, int z): x(x), y(y), z(z) {}

    Vector operator+ (const Vector& other) {
        Vector result;
        result.x = this->x + other.x;
        result.y = this->y + other.y;
        result.z = this->z + other.z;
        return result;
    }

    Vector& operator = (const Vector& other) {
        if (this != &other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
        }

        return *this;
    }

    friend std::ostream& operator << (std::ostream& output, const Vector& vector) {
        output << vector.x << ' ' << vector.y << ' ' << vector.z << ' ';
        return output;
    }

    friend std::istream& operator >> (std::istream& input, Vector& vector) {
        input >> vector.x;
        input >> vector.y;
        input >> vector.z;
        return input;
    }
};
