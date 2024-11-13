#pragma once
#include "../math.h"

namespace phys {
	template <typename T>
	concept bounding_volume = requires(T t, const T ct) {
		{ T(ct, ct) } -> std::convertible_to<T>;
		{ ct.overlaps(ct) } -> std::convertible_to<bool>;
		{ ct.growth(ct) } -> std::convertible_to<real>;
		{ ct == ct } -> std::convertible_to<bool>;
	} && std::default_initializable<T>;

	struct bounding_sphere {
		vec3 center{};
		real radius{};

		bounding_sphere() = default;
		bounding_sphere(const vec3 &_center, real _radius);
		bounding_sphere(const bounding_sphere &a, const bounding_sphere &b);

		bool overlaps(const bounding_sphere &other) const;
		real growth(const bounding_sphere &other) const;

		friend bool operator==(const bounding_sphere &a, const bounding_sphere &b);

	private:
		real volume() const;
	};

	static_assert(bounding_volume<bounding_sphere>);
}
