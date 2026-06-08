#pragma once

#include <functional>
#include <string>

namespace alphabet {

class VoiceInput {
  public:
    VoiceInput();
    ~VoiceInput();

    // Start the voice server process
    bool start(const std::string& python_path = "python3");

    // Initialize STT for a language (en/es/fr/de/am)
    bool init_language(const std::string& lang);

    // Listen for speech and return transcribed text
    // timeout: max seconds to listen
    std::string listen(int timeout = 10);

    // Check if voice is available
    bool is_available() const;

    // Stop the voice server
    void stop();

    // Get status info
    std::string get_status();

  private:
    bool send_command(const std::string& json_cmd);
    std::string read_response();

    int stdin_fd_ = -1;
    int stdout_fd_ = -1;
    int pid_ = -1;
    bool running_ = false;
    std::string current_lang_;
};

} // namespace alphabet
