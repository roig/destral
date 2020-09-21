#pragma once



namespace ds {
	/** Those are the unique math types that will be used in the ds Engine and are able to reflect etc..*/
	// float we will use standard float
	// bool we will use standard bool
	using S32 = Magnum::Int;
	using Vec2 = Magnum::Vector2;
	using Vec2i = Magnum::Vector2i;
	using Vec4 = Magnum::Vector4;
	using Mat3 = Magnum::Matrix3;
	using Color = Magnum::Color4;
	using Rect = Magnum::Range2D;


	inline std::string to_string(const Vec2& v) {
		return "{ " + std::to_string(v.x()) + ", " + std::to_string(v.y()) + " }";
	}

	inline std::string to_string(const Vec2i& v) {
		return "{ " + std::to_string(v.x()) + ", " + std::to_string(v.y()) + " }";
	}

	inline std::string to_string(const Vec4& v) {
		return "{ " + std::to_string(v.x()) + ", " + std::to_string(v.y()) + ", " + std::to_string(v.z()) + ", " + std::to_string(v.w()) + " }";
	}

	inline std::string to_string(const Mat3& v) {
		std::string s;
		return s;
	}

	inline std::string to_string(const Color& v) {
		return "{ " + std::to_string(v.x()) + ", " + std::to_string(v.y()) + ", " + std::to_string(v.z()) + ", " + std::to_string(v.w()) + " }";
	}

	inline std::string to_string(const Rect& v) {
		std::string s;
		return s;
	}
	inline std::string to_string(bool v) {
		return v? "true": "false";
	}

}
