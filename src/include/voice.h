#pragma once

#include <functional>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace alphabet {

class VoiceInput {
  public:
    VoiceInput();
    ~VoiceInput();

    bool start(const std::string& python_path = "python3");
    bool init_language(const std::string& lang);
    std::string listen(int timeout = 10);
    bool is_available() const;
    void stop();
    std::string get_status();

  private:
    bool send_command(const std::string& json_cmd);
    std::string read_response();

#ifdef _WIN32
    HANDLE h_stdin_ = INVALID_HANDLE_VALUE;
    HANDLE h_stdout_ = INVALID_HANDLE_VALUE;
    HANDLE h_process_ = INVALID_HANDLE_VALUE;
#else
    int stdin_fd_ = -1;
    int stdout_fd_ = -1;
    int pid_ = -1;
#endif
    bool running_ = false;
    std::string current_lang_;
};

} // namespace alphabet
