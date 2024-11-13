#include <Windows.h>
#include <gdiplus.h>
#include <cassert>
#include <cstring>
#include <string>
#include "gl.h"
#include "texture.h"

texture::texture(const char * const path, bool generate_mipmap) :
	id(0, [](unsigned int _handle) {
		glDeleteTextures(1, &_handle);
	})
{
	size_t path_size = strlen(path);
	std::wstring wc_path(path_size + 1, L'#');
	size_t num_converted;
	mbstowcs_s(&num_converted, &wc_path[0], path_size + 1, path, path_size);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Gdiplus::Bitmap tex_bmp(wc_path.c_str());
	tex_bmp.GetPixelFormat();

	dimensions.x = tex_bmp.GetWidth();
	dimensions.y = tex_bmp.GetHeight();

	Gdiplus::Rect clip(0, 0, dimensions.x, dimensions.y);
	Gdiplus::BitmapData data;

	int internal_gl_format;
	int external_gl_format;
	const int gdi_format = tex_bmp.GetPixelFormat();

	switch (gdi_format) {
		case PixelFormat24bppRGB: {
			internal_gl_format = GL_RGB;
			external_gl_format = GL_BGR;
			break;
		}
		case PixelFormat32bppARGB: {
			internal_gl_format = GL_RGBA;
			external_gl_format = GL_BGRA;
			break;
		}
		default: {
			throw "Unsupported pixel format";
		}
	}

	tex_bmp.LockBits(&clip, Gdiplus::ImageLockMode::ImageLockModeRead, gdi_format, &data);

	assert(("BitmapData width is correct", dimensions.x == data.Width));
	assert(("BitmapData height is correct", dimensions.y == data.Height));
	assert(("BitmapData PixelFormat is correct", data.PixelFormat == gdi_format));

	unsigned char * buf = new unsigned char[data.Stride * data.Height];

	for (unsigned int i = 0; i < data.Height; i++) {
		memcpy(buf + data.Stride * (data.Height - i - 1), (unsigned char *)data.Scan0 + data.Stride * i, data.Stride);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internal_gl_format, dimensions.x, dimensions.y, 0, external_gl_format, GL_UNSIGNED_BYTE, buf);

	if (generate_mipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	delete[] buf;

	tex_bmp.UnlockBits(&data);
}

unsigned int texture::get_id() const {
	return id;
}

const glm::uvec2& texture::get_dimensions() const {
	return dimensions;
}

bool operator==(const texture &a, const texture &b) {
	return a.get_id() == b.get_id();
}
