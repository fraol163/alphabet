#include "voice.h"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

namespace alphabet {

VoiceInput::VoiceInput() {}

VoiceInput::~VoiceInput() {
    stop();
}

bool VoiceInput::start(const std::string& python_path) {
    if (running_)
        return true;

    // Find voice_server.py relative to binary
    std::string script_path = "tools/voice_server.py";

    // Try relative to binary location
    char bin_path[4096] = {};
    ssize_t len = readlink("/proc/self/exe", bin_path, sizeof(bin_path) - 1);
    if (len > 0) {
        bin_path[len] = '\0';
        std::string bin_dir(bin_path);
        auto pos = bin_dir.rfind('/');
        if (pos != std::string::npos) {
            bin_dir = bin_dir.substr(0, pos);
            std::string candidate = bin_dir + "/../tools/voice_server.py";
            if (access(candidate.c_str(), F_OK) == 0) {
                script_path = candidate;
            }
        }
    }

    // Create pipes for communication
    int child_stdin[2];  // parent writes, child reads
    int child_stdout[2]; // child writes, parent reads

    if (pipe(child_stdin) < 0 || pipe(child_stdout) < 0) {
        return false;
    }

    pid_ = fork();
    if (pid_ < 0) {
        close(child_stdin[0]);
        close(child_stdin[1]);
        close(child_stdout[0]);
        close(child_stdout[1]);
        return false;
    }

    if (pid_ == 0) {
        // Child process
        close(child_stdin[1]);
        close(child_stdout[0]);

        dup2(child_stdin[0], STDIN_FILENO);
        dup2(child_stdout[1], STDOUT_FILENO);

        close(child_stdin[0]);
        close(child_stdout[1]);

        execlp(python_path.c_str(), python_path.c_str(), script_path.c_str(), nullptr);
        _exit(127);
    }

    // Parent process
    close(child_stdin[0]);
    close(child_stdout[1]);

    stdin_fd_ = child_stdin[1];
    stdout_fd_ = child_stdout[0];

    // Set non-blocking for timeout handling
    fcntl(stdout_fd_, F_SETFL, O_NONBLOCK);

    running_ = true;

    // Wait for server ready message
    std::string response = read_response();
    return !response.empty();
}

bool VoiceInput::send_command(const std::string& json_cmd) {
    if (!running_)
        return false;

    std::string cmd = json_cmd + "\n";
    ssize_t written = write(stdin_fd_, cmd.c_str(), cmd.size());
    return written == (ssize_t)cmd.size();
}

std::string VoiceInput::read_response() {
    if (!running_)
        return "";

    // Read with timeout
    char buf[4096];
    std::string result;
    int attempts = 0;
    int max_attempts = 300; // 30 seconds with 100ms sleep

    while (attempts < max_attempts) {
        ssize_t n = read(stdout_fd_, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            result += buf;
            // Check if we have a complete JSON line
            if (result.find('\n') != std::string::npos) {
                break;
            }
        } else if (n == 0) {
            break; // EOF
        } else {
            usleep(100000); // 100ms
            attempts++;
        }
    }

    // Trim to first complete line
    auto pos = result.find('\n');
    if (pos != std::string::npos) {
        result = result.substr(0, pos);
    }

    return result;
}

bool VoiceInput::init_language(const std::string& lang) {
    current_lang_ = lang;
    std::string cmd = "{\"cmd\": \"init\", \"lang\": \"" + lang + "\"}";
    if (!send_command(cmd))
        return false;

    std::string response = read_response();
    return response.find("\"ready\"") != std::string::npos;
}

std::string VoiceInput::listen(int timeout) {
    std::string cmd = "{\"cmd\": \"listen\", \"timeout\": " + std::to_string(timeout) + "}";
    if (!send_command(cmd))
        return "";

    std::string response = read_response();

    // Parse JSON response
    // Simple parser: look for "text": "..."
    auto text_pos = response.find("\"text\"");
    if (text_pos == std::string::npos)
        return "";

    auto colon_pos = response.find(":", text_pos);
    if (colon_pos == std::string::npos)
        return "";

    auto quote_start = response.find("\"", colon_pos + 1);
    if (quote_start == std::string::npos)
        return "";

    auto quote_end = response.find("\"", quote_start + 1);
    if (quote_end == std::string::npos)
        return "";

    return response.substr(quote_start + 1, quote_end - quote_start - 1);
}

bool VoiceInput::is_available() const {
    return running_;
}

void VoiceInput::stop() {
    if (!running_)
        return;

    send_command("{\"cmd\": \"quit\"}");
    usleep(500000); // 500ms

    if (stdin_fd_ >= 0) {
        close(stdin_fd_);
        stdin_fd_ = -1;
    }
    if (stdout_fd_ >= 0) {
        close(stdout_fd_);
        stdout_fd_ = -1;
    }

    if (pid_ > 0) {
        kill(pid_, SIGTERM);
        waitpid(pid_, nullptr, 0);
        pid_ = -1;
    }

    running_ = false;
}

std::string VoiceInput::get_status() {
    if (!running_)
        return "Voice server not running";

    send_command("{\"cmd\": \"status\"}");
    return read_response();
}

} // namespace alphabet
