#include "../stdafx.h"
#include "Window.hpp"

#include "stb_image.h"

// Store last GLFW error in a thread-local so we can throw safely outside the callback
namespace {
    thread_local int         g_lastErrCode = 0;
    thread_local std::string g_lastErrMsg;

    // Small helper to throw with the last GLFW error (if any)
    [[noreturn]] void throwGlfwError(const char* prefix)
    {
        std::string what = prefix ? std::string(prefix) + ": " : "";
        if (!g_lastErrMsg.empty()) what += g_lastErrMsg;
        else                       what += "Unknown GLFW error";
        throw std::runtime_error(what);
    }
}

Window::Window()
{
    width_ = Config::width;
    height_ = Config::height;
    title_ = Config::window_name;

    initGlfw();
    createWindow();
    loadGL();
}

Window::~Window()
{
    if (handle_) {
        glfwDestroyWindow(handle_);
        handle_ = nullptr;
    }
    glfwTerminate();
}

// -----------------------------
// Public API //
// -----------------------------
bool Window::ShouldClose() const
{
    return handle_ ? glfwWindowShouldClose(handle_) != 0 : true;
}

void Window::SetCloseFlag()
{
    if (handle_) glfwSetWindowShouldClose(handle_, 1);
}

GLFWwindow* Window::GetGLFWWindow() const
{
    return handle_;
}

bool Window::Resized() const
{
    return resized_;
}

int Window::GetWidth() const
{
    return width_;
}

int Window::GetHeight() const
{
    return height_;
}

void Window::ResetResizedFlag()
{
    resized_ = false;
}

void Window::SetWindowIcon(const char* path)
{
    int w = 0, h = 0, channels = 0;
    unsigned char* pixels = stbi_load(path, &w, &h, &channels, 4); // RGBA
    if (!pixels) return;

    GLFWimage img;
    img.width = w;
    img.height = h;
    img.pixels = pixels;

    glfwSetWindowIcon(handle_, 1, &img);
    stbi_image_free(pixels);
}

// Convenience (optional)
void Window::MakeContextCurrent()
{
    if (handle_) glfwMakeContextCurrent(handle_);
}
void Window::SwapBuffers()
{
    if (handle_) glfwSwapBuffers(handle_);
}
void Window::SetVSync(bool on)
{
    glfwSwapInterval(on ? 1 : 0);
}

// -----------------------------
// Callbacks
// -----------------------------
void Window::OnError(int error, const char* description)
{
    g_lastErrCode = error;
    g_lastErrMsg = description ? description : "";
    // If you want to log the result: Logger::Log(std::string("[GLFW] ") + g_lastErrMsg);
}

void Window::OnFramebufferResized(GLFWwindow* window, int width, int height)
{
    if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
        self->width_ = width;
        self->height_ = height;
        self->resized_ = true;
    }
}

// -----------------------------
// Init stages
// -----------------------------
void Window::initGlfw()
{
    g_lastErrCode = 0;
    g_lastErrMsg.clear();

    glfwSetErrorCallback(&Window::OnError);

    if (!glfwInit()) {
        throwGlfwError("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
}

void Window::createWindow()
{
    handle_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (!handle_) {
        throwGlfwError("Failed to create GLFW window");
    }

    // set runtime icon (ICO works; stb_image can read .ico)
    SetWindowIcon("assets/icons/billiards.ico");

    glfwMakeContextCurrent(handle_);
    glfwSwapInterval(1); // vsync on by default

    // route callbacks to this instance
    glfwSetWindowUserPointer(handle_, this);
    glfwSetWindowSizeCallback(handle_, &Window::OnFramebufferResized);
}

void Window::loadGL()
{
    // gladLoadGL returns a function pointer; we check success instead
    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("Failed to load OpenGL functions (gladLoadGL)");
    }
}

