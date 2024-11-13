#pragma once
#include <functional>

// This is like `unique_ptr`, but for handles. A unique handle is constructed with a null value
// and a deleter function that specifies how to delete the object referenced by the handle. On
// construction, the `unique_handle` has no object, so its handle is the null handle. If a
// construction function expects a pointer which will receive the value of a newly created handle,
// the address of the `unique_handle` can be used, and it will be converted to the expected handle
// type. A `unique_handle` has value semantics:
//		1. It will automatically cast to the handle type when treated as such
//		2. Assigning the null handle value to a `unique_handle` will free the held object, if any
//		3. When appropriate, the address of the handle can be taken by taking the address of the
//			`unique_handle`
//		4. It can be compared for equality with other `unique_handle`s, and the held handles will
//			be compared
// For example, to create an OpenGL buffer:
//
//		unique_handle<unsigned int> vbo(0, [](unsigned int _vbo) {
//			glDeleteBuffers(1, &_vbo);
//		});
//		glGenBuffers(1, &vbo); // The address of the handle itself is taken, not the `unique_handle`
//		glBindBuffer(GL_ARRAY_BUFFER, vbo); // The `unique_handle` is implicitly converted to the handle type
//		...
template <typename T>
class unique_handle {
public:
	using deleter = std::function<void(T)>;

	// TODO: Error handling
	unique_handle(T _null_handle, deleter _delete_handle) noexcept;

	unique_handle(const unique_handle<T> &other) = delete;
	unique_handle(unique_handle<T> &&other) noexcept;

	unique_handle<T>& operator=(const unique_handle<T> &other) = delete;
	// TODO: Error handling. What if a GL call fails here
	unique_handle<T>& operator=(unique_handle<T> &&other) noexcept;

	operator T() noexcept;
	operator T() const noexcept;

	T * operator&() noexcept;
	const T * operator&() const noexcept;

	// Assigns to the handle held within. If a null handle is provided
	// and the held handle is not null, the held handle will be deleted.
	T& operator=(T other) noexcept;

	friend bool operator==(const unique_handle<T> &a, const unique_handle<T> &b) {
		return a.handle == b.handle;
	}

	~unique_handle();
private:
	T handle;
	T null_handle;
	deleter delete_handle;
};

template <typename T>
unique_handle<T>::unique_handle(T _null_handle, deleter _delete_handle) noexcept :
	handle(_null_handle),
	null_handle(_null_handle),
	delete_handle(_delete_handle)
{}

template <typename T>
unique_handle<T>::unique_handle(unique_handle<T> &&other) noexcept {
	handle = other.handle;
	null_handle = other.null_handle;
	delete_handle = other.delete_handle;

	other.handle = other.null_handle;
}

template <typename T>
unique_handle<T>& unique_handle<T>::operator=(unique_handle<T> &&other) noexcept {
	if (handle != null_handle) {
		delete_handle(handle);
		handle = null_handle;
	}

	handle = other.handle;
	null_handle = other.null_handle;
	delete_handle = other.delete_handle;

	other.handle = other.null_handle;

	return *this;
}

template <typename T>
unique_handle<T>::~unique_handle() {
	if (handle != null_handle) {
		delete_handle(handle);
	}
}

template <typename T>
unique_handle<T>::operator T() noexcept {
	return handle;
}

template <typename T>
unique_handle<T>::operator T() const noexcept {
	return handle;
}

template <typename T>
T * unique_handle<T>::operator&() noexcept {
	return &handle;
}

template <typename T>
const T * unique_handle<T>::operator&() const noexcept {
	return &handle;
}

template <typename T>
T& unique_handle<T>::operator=(T other) noexcept {
	if (other == null_handle && handle != null_handle) {
		delete_handle(handle);
	}

	handle = other;

	return handle;
}
