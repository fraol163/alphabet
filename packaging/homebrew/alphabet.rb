class Alphabet < Formula
  desc "Multilingual programming language — code in English, Amharic, Spanish, French, or German"
  homepage "https://github.com/alphabet-lang/alphabet"
  url "https://github.com/alphabet-lang/alphabet/archive/v2.3.5.tar.gz"
  sha256 "PLACEHOLDER"
  license "MIT"
  head "https://github.com/alphabet-lang/alphabet.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "nlohmann-json" => :build

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "bin"

    # Install stdlib
    pkgshare.install "stdlib"

    # Install examples
    pkgshare.install "examples"

    # Install learn
    pkgshare.install "learn"
  end

  test do
    assert_match "Alphabet #{version}", shell_output("#{bin}/alphabet --version")

    # Test hello world
    (testpath/"hello.abc").write <<~EOS
      #alphabet<en>
      z.o("Hello, World!")
    EOS
    assert_match "Hello, World!", shell_output("#{bin}/alphabet run hello.abc")
  end
end
