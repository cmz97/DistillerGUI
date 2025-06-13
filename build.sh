#!/bin/bash

# Exit on any error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Detect operating system
detect_os() {
	case "$(uname -s)" in
	Linux*)
		if [ -f /etc/arch-release ]; then
			echo "ArchLinux"
		elif [ -f /etc/debian_version ]; then
			echo "Debian"
		else
			echo "Linux"
		fi
		;;
	Darwin*) echo "macOS" ;;
	*) echo "Unknown" ;;
	esac
}

OS=$(detect_os)
echo -e "${BLUE}Detected OS: $OS${NC}"

# Function to install dependencies based on OS
install_dependencies() {
	case $OS in
	"ArchLinux")
		echo -e "${YELLOW}Installing dependencies for Arch Linux...${NC}"
		if command -v yay >/dev/null 2>&1; then
			yay -S --needed base-devel cmake git libgpiod
		elif command -v paru >/dev/null 2>&1; then
			paru -S --needed base-devel cmake git libgpiod
		else
			pacman -S --needed base-devel cmake git libgpiod
		fi
		;;
	"Debian")
		echo -e "${YELLOW}Installing dependencies for Debian/Ubuntu...${NC}"
		apt-get update
		apt-get install -y build-essential cmake git libgpiod-dev gpiod
		;;
	"macOS")
		echo -e "${YELLOW}Installing dependencies for macOS...${NC}"
		if command -v brew >/dev/null 2>&1; then
			brew install cmake git
			echo -e "${YELLOW}Note: GPIO functionality may not be available on macOS${NC}"
		else
			echo -e "${RED}Error: Homebrew not found. Please install Homebrew first.${NC}"
			echo -e "${BLUE}Visit: https://brew.sh/${NC}"
			exit 1
		fi
		;;
	*)
		echo -e "${YELLOW}Unknown OS. Please install dependencies manually:${NC}"
		echo -e "${BLUE}- build-essential or equivalent compiler tools${NC}"
		echo -e "${BLUE}- cmake${NC}"
		echo -e "${BLUE}- git${NC}"
		echo -e "${BLUE}- libgpiod-dev (if available)${NC}"
		;;
	esac
}

# Check if we should install dependencies
INSTALL_DEPS=false
if [ "$1" = "--install-deps" ] || [ "$1" = "-i" ]; then
	INSTALL_DEPS=true
fi

# Install dependencies if requested and we have appropriate permissions
if [ "$INSTALL_DEPS" = true ]; then
	if [ "$OS" = "macOS" ]; then
		install_dependencies
	elif [ "$EUID" -eq 0 ]; then
		install_dependencies
	else
		echo -e "${YELLOW}Installing dependencies requires root privileges. Re-running with sudo...${NC}"
		exec sudo "$0" "$@"
	fi
elif [ "$OS" != "macOS" ] && [ "$EUID" -ne 0 ] && [ "$1" != "--no-deps" ]; then
	echo -e "${YELLOW}To automatically install dependencies, run: $0 --install-deps${NC}"
	echo -e "${YELLOW}To skip dependency check, run: $0 --no-deps${NC}"
fi

echo -e "${YELLOW}Building DistillerGUI...${NC}"

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
	echo -e "${RED}Error: CMakeLists.txt not found. Are you in the right directory?${NC}"
	echo "Please run this script from the DistillerGUI root directory"
	exit 1
fi

# Check for required tools
echo -e "${YELLOW}Checking for required tools...${NC}"
command -v cmake >/dev/null 2>&1 || {
	echo -e "${RED}Error: cmake is required but not installed.${NC}" >&2
	echo -e "${BLUE}Install with: $0 --install-deps${NC}"
	exit 1
}

# Check for build tool based on OS
if [ "$OS" = "macOS" ]; then
	# macOS can use make or Xcode
	if ! command -v make >/dev/null 2>&1 && ! command -v xcodebuild >/dev/null 2>&1; then
		echo -e "${RED}Error: Neither make nor Xcode build tools found.${NC}" >&2
		echo -e "${BLUE}Install Xcode Command Line Tools: xcode-select --install${NC}"
		exit 1
	fi
else
	# Linux and other Unix-like systems typically use make
	command -v make >/dev/null 2>&1 || {
		echo -e "${RED}Error: make is required but not installed.${NC}" >&2
		echo -e "${BLUE}Install with: $0 --install-deps${NC}"
		exit 1
	}
fi

# Check for required files
echo -e "${YELLOW}Checking for required files...${NC}"
required_files=("main.c" "eink_driver.c" "eink_driver.h" "lv_conf.h")
for file in "${required_files[@]}"; do
	if [ ! -f "$file" ]; then
		echo -e "${RED}Error: Required file $file not found${NC}"
		exit 1
	fi
done

# Clean build directory if it exists
echo -e "${YELLOW}Cleaning build directory...${NC}"
rm -rf build

# Create new build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure and build
echo -e "${YELLOW}Configuring with CMake...${NC}"

# Set CMake arguments based on OS
CMAKE_ARGS=""
BUILD_CMD=""

case $OS in
"macOS")
	# For macOS, use Unix Makefiles or Xcode
	if command -v make >/dev/null 2>&1; then
		CMAKE_ARGS="-G \"Unix Makefiles\""
		BUILD_CMD="make -j$(sysctl -n hw.ncpu)"
	else
		CMAKE_ARGS="-G \"Xcode\""
		BUILD_CMD="cmake --build . --config Release"
	fi
	;;
*)
	# Linux and other Unix-like systems
	CMAKE_ARGS="-G \"Unix Makefiles\""
	if command -v nproc >/dev/null 2>&1; then
		BUILD_CMD="make -j$(nproc)"
	else
		BUILD_CMD="make -j4" # fallback to 4 cores
	fi
	;;
esac

# Run CMake configuration
if eval "cmake .. $CMAKE_ARGS"; then
	echo -e "${GREEN}CMake configuration successful${NC}"
else
	echo -e "${RED}CMake configuration failed${NC}"
	exit 1
fi

echo -e "${YELLOW}Building with: $BUILD_CMD${NC}"
if eval "$BUILD_CMD"; then
	echo -e "${GREEN}Build successful!${NC}"
else
	echo -e "${RED}Build failed${NC}"
	exit 1
fi

# Check if the binary was created (different locations based on OS and build system)
BINARY_FOUND=false
BINARY_PATH=""

if [ -f "DistillerGUI" ]; then
	BINARY_PATH="build/DistillerGUI"
	BINARY_FOUND=true
elif [ -f "DistillerGUI.exe" ]; then
	BINARY_PATH="build/DistillerGUI.exe"
	BINARY_FOUND=true
elif [ -f "Release/DistillerGUI.exe" ]; then
	BINARY_PATH="build/Release/DistillerGUI.exe"
	BINARY_FOUND=true
elif [ -f "Debug/DistillerGUI.exe" ]; then
	BINARY_PATH="build/Debug/DistillerGUI.exe"
	BINARY_FOUND=true
fi

if [ "$BINARY_FOUND" = true ]; then
	echo -e "${GREEN}Build completed successfully!${NC}"
	echo -e "${GREEN}Binary created at: $BINARY_PATH${NC}"

	# Show usage instructions
	echo -e "${BLUE}Usage instructions:${NC}"
	case $OS in
	"macOS" | "Linux" | *)
		echo -e "${BLUE}- Run: ./$BINARY_PATH${NC}"
		echo -e "${BLUE}- Make executable: chmod +x $BINARY_PATH${NC}"
		;;
	esac
else
	echo -e "${RED}Binary not found after build${NC}"
	echo -e "${YELLOW}Searched for:${NC}"
	echo -e "${YELLOW}- DistillerGUI${NC}"
	echo -e "${YELLOW}- DistillerGUI.exe${NC}"
	echo -e "${YELLOW}- Release/DistillerGUI.exe${NC}"
	echo -e "${YELLOW}- Debug/DistillerGUI.exe${NC}"
	exit 1
fi

cd ..

# Display final message with helpful information
echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}     DistillerGUI Build Complete       ${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "${BLUE}Platform Support:${NC}"
echo -e "${BLUE}✓ Linux (Debian/Ubuntu/Arch)${NC}"
echo -e "${BLUE}✓ macOS (with Homebrew)${NC}"
echo ""
echo -e "${BLUE}Build Script Options:${NC}"
echo -e "${BLUE}  $0                    # Build only${NC}"
echo -e "${BLUE}  $0 --install-deps     # Install dependencies and build${NC}"
echo -e "${BLUE}  $0 --no-deps          # Skip dependency checks${NC}"
echo ""

# Add help function for future reference
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
	echo -e "${YELLOW}DistillerGUI Build Script Help${NC}"
	echo ""
	echo -e "${BLUE}USAGE:${NC}"
	echo -e "  $0 [OPTIONS]"
	echo ""
	echo -e "${BLUE}OPTIONS:${NC}"
	echo -e "  --install-deps, -i    Install system dependencies before building"
	echo -e "  --no-deps            Skip dependency installation and checks"
	echo -e "  --help, -h           Show this help message"
	echo ""
	echo -e "${BLUE}SUPPORTED PLATFORMS:${NC}"
	echo -e "  • Linux (Debian/Ubuntu): apt-get install build-essential cmake git libgpiod-dev"
	echo -e "  • Linux (Arch): pacman -S base-devel cmake git libgpiod"
	echo -e "  • macOS: brew install cmake git"
	echo ""
	echo -e "${BLUE}EXAMPLES:${NC}"
	echo -e "  $0 --install-deps     # Fresh install with dependencies"
	echo -e "  sudo $0 --install-deps # On Linux, install system packages"
	echo -e "  $0                    # Build only (dependencies already installed)"
	echo ""
	exit 0
fi
