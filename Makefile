# Makefile for Redbird Libimage Engine

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra \
           -I. \
           -Isrc/Game \
           -Isrc/Math \
           -Isrc/Physics \
           -Isrc/Render \
           -I/opt/homebrew/include \
           -DGL_SILENCE_DEPRECATION

LDFLAGS = -L/opt/homebrew/lib \
          -framework OpenGL \
          -lglfw \
          -lGLEW \
          -lm \
          -llua

SOURCES = src/main.cpp \
          src/Game/Workspace.cpp \
          src/Game/GameData.cpp \
          src/Physics/Physics.cpp \
          src/Render/Renderer.cpp \
          src/Render/Shader.cpp \
          src/Game/ScriptRunner.cpp

OBJECTS = $(SOURCES:.cpp=.o)
TARGET = engine

# 色付き出力
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

# デフォルトターゲット
all: $(TARGET)
	@echo "$(GREEN)✓ Build complete: $(TARGET)$(NC)"

# 実行ファイルのビルド
$(TARGET): $(OBJECTS)
	@echo "$(BLUE)Linking...$(NC)"
	@$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

# オブジェクトファイルのビルド
%.o: %.cpp
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# クリーンアップ
clean:
	@rm -f $(OBJECTS) $(TARGET)
	@echo "$(GREEN)✓ Clean complete$(NC)"

# 再ビルド
rebuild: clean all

# ビルドして実行
run: $(TARGET)
	@echo "$(BLUE)Running $(TARGET)...$(NC)"
	@./$(TARGET)

# 高速リビルド＆実行（よく使う！）
r: rebuild run

# デバッグビルド（最適化なし、デバッグシンボル付き）
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: clean $(TARGET)
	@echo "$(GREEN)✓ Debug build complete$(NC)"

# リリースビルド（最適化あり）
release: CXXFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)
	@echo "$(GREEN)✓ Release build complete$(NC)"

# 依存関係の表示
info:
	@echo "$(BLUE)Project Info:$(NC)"
	@echo "  Compiler: $(CXX)"
	@echo "  Target: $(TARGET)"
	@echo "  Sources: $(words $(SOURCES)) files"
	@echo "  Objects: $(OBJECTS)"

# ヘルプ
help:
	@echo "$(BLUE)Available targets:$(NC)"
	@echo "  $(GREEN)make$(NC)          - Build the project"
	@echo "  $(GREEN)make run$(NC)      - Build and run"
	@echo "  $(GREEN)make r$(NC)        - Quick rebuild and run"
	@echo "  $(GREEN)make clean$(NC)    - Remove build files"
	@echo "  $(GREEN)make rebuild$(NC)  - Clean and build"
	@echo "  $(GREEN)make debug$(NC)    - Build with debug symbols"
	@echo "  $(GREEN)make release$(NC)  - Build optimized version"
	@echo "  $(GREEN)make info$(NC)     - Show project info"
	@echo "  $(GREEN)make help$(NC)     - Show this help"

.PHONY: all clean rebuild run r debug release info help