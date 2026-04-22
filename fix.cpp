    size_t lang_length = close_pos - lang_start;
    language_ = std::string(header_source.substr(lang_start, lang_length));

    size_t newline_pos = header_source.find('\n', close_pos);
    if (newline_pos != std::string_view::npos) {
        current_ = current_ + newline_pos + 1;
        start_ = current_;
        line_ = 2;
        column_ = 0;
    }
}
