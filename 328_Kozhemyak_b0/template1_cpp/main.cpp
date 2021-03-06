#include "common.h"
#include "Image.h"
#include "Player.h"
#include <tuple>
#include <fstream>
#include <sys/select.h>
#include <unistd.h>

#define GLFW_DLL
#include <GLFW/glfw3.h>

typedef std::vector<std::vector<char>> textt;
constexpr GLsizei WINDOW_WIDTH = 1000, WINDOW_HEIGHT = 800;
Point starting_pos;


struct InputState
{
  bool keys[1024]{}; //массив состояний кнопок - нажата/не нажата
  GLfloat lastX = 400, lastY = 300; //исходное положение мыши
  bool firstMouse = true;
  bool captureMouse = true;  // Мышка захвачена нашим приложением или нет?
  bool capturedMouseJustNow = false;
} static Input;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
  case GLFW_KEY_1:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    break;
  case GLFW_KEY_2:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    break;
	default:
		if (action == GLFW_PRESS)
      Input.keys[key] = true;
		else if (action == GLFW_RELEASE)
      Input.keys[key] = false;
	}
}

void processPlayerMovement(Player &player)
{
  if (Input.keys[GLFW_KEY_W])
    player.ProcessInput(MovementDir::UP);
  else if (Input.keys[GLFW_KEY_S])
    player.ProcessInput(MovementDir::DOWN);
  if (Input.keys[GLFW_KEY_A])
    player.ProcessInput(MovementDir::LEFT);
  else if (Input.keys[GLFW_KEY_D])
    player.ProcessInput(MovementDir::RIGHT);
}


void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    Input.captureMouse = !Input.captureMouse;

  if (Input.captureMouse)
  {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Input.capturedMouseJustNow = true;
  }
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (Input.firstMouse)
  {
    Input.lastX = float(xpos);
    Input.lastY = float(ypos);
    Input.firstMouse = false;
  }

  GLfloat xoffset = float(xpos) - Input.lastX;
  GLfloat yoffset = Input.lastY - float(ypos);

  Input.lastX = float(xpos);
  Input.lastY = float(ypos);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
  // ...
}


int initGL()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  std::cout << "Controls: "<< std::endl;
  std::cout << "press right mouse button to capture/release mouse cursor  "<< std::endl;
  std::cout << "W, A, S, D - movement  "<< std::endl;
  std::cout << "press ESC to exit" << std::endl;

	return 0;
}


#define N  4 
#define M  5
char all_rooms[N][M]; 

std::string get_hall(char ch, Hall *room);

void read_map(std::string path);

textt read_file(std::string path, Image &img, Hall *room); 
textt change_file(std::string path, Player &player);
void DrawPrize(int counter, Image &img);




int main(int argc, char** argv) {
  int counter_prize = 0;
	if(!glfwInit())
    return -1;

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow*  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "task1 base project", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); 

	glfwSetKeyCallback        (window, OnKeyboardPressed);  
	glfwSetCursorPosCallback  (window, OnMouseMove); 
  glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback     (window, OnMouseScroll);

	if(initGL() != 0) 
		return -1;
	
  //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();
  

	Image screenBuffer(WINDOW_WIDTH, WINDOW_HEIGHT, 4);
  Image background(WINDOW_WIDTH, WINDOW_HEIGHT, 4);
  Image gameOver("../resources/game_over.jpg");
  Image win("../resources/win.jpg");
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);  GL_CHECK_ERRORS;
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); GL_CHECK_ERRORS;

  

  read_map("../resources/map.txt");

  
  // Инициализируем лабиринт
  Hall one_room[N][M];
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      if (i > 0)
        one_room[i][j].up = &one_room[i-1][j];
      if (i < N-1)
        one_room[i][j].down = &one_room[i+1][j];
      if (j > 0)
        one_room[i][j].left = &one_room[i][j-1];
      if (j < M-1)
        one_room[i][j].right = &one_room[i][j+1];

      char ch = all_rooms[i][j];
      if (ch >= 'A' && ch <= 'Z') {
        one_room[i][j].type = ch;

          one_room[i][j].path = "../resources/floor.png";
          one_room[i][j].tile_bias_x = 0;
          one_room[i][j].tile_bias_y = 0;
          
      }
    }
  }


  Hall *current_room = &one_room[0][0]; // ставим текущую комнату на начальную

  textt tmp_txt = read_file("../resources/room1.txt", screenBuffer, current_room);
  read_file("../resources/room1.txt", background, current_room);
  Player player{starting_pos};
  player.save_room(tmp_txt);

  int p = 0;


  //game loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
    glfwPollEvents();

    player.cum_time_inc(deltaTime); 
    processPlayerMovement(player);
    player.Draw(screenBuffer, background, current_room); 
    
    int tmp;
  
    if ( tmp = player.isDoor() ) {

      if (current_room->door[tmp-1]) {
        switch (tmp)                    
        {
        case 1:
          current_room = current_room->up;
          player.set_coords(12 * tileSize, 2 * tileSize); 
          break;
        case 2:
          current_room = current_room->down;
          player.set_coords(12 * tileSize, 22 * tileSize);
          break;
        case 3:
          current_room = current_room->left;
          player.set_coords(22 * tileSize, 12 * tileSize);
          break;
        case 4:
          current_room = current_room->right;
          player.set_coords(2 * tileSize, 12 * tileSize);
          break;
        default:
          break;
        }
    
        tmp_txt = read_file(get_hall(current_room->type, current_room), screenBuffer, current_room);
        read_file(get_hall(current_room->type, current_room), background, current_room);
        player.save_room(tmp_txt);
    
      }
    }

    if (player.isPrize()) {
      counter_prize++;
      tmp_txt = change_file(get_hall(current_room->type, current_room), player);
      tmp_txt = read_file(get_hall(current_room->type, current_room), screenBuffer, current_room);
      read_file(get_hall(current_room->type, current_room), background, current_room);
      player.save_room(tmp_txt);
      player.Draw(screenBuffer, background, current_room); 
    }
    
    if (player.isEmpty()) {
      for (int y = 0; y < WINDOW_HEIGHT; y++)
        for (int x = 0; x < WINDOW_WIDTH; x++) 
          screenBuffer.PutPixel(x, y, gameOver.GetPixel(x, WINDOW_HEIGHT - y - 1));
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;
      glDrawPixels (WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data()); GL_CHECK_ERRORS;
      glfwSwapBuffers(window);
      sleep(5);
      return 0;
    }

    DrawPrize(counter_prize, screenBuffer);

    if (player.isQuit()) {
      for (int y = 0; y < WINDOW_HEIGHT; y++)
        for (int x = 0; x < WINDOW_WIDTH; x++) 
          screenBuffer.PutPixel(x, y, win.GetPixel(x, WINDOW_HEIGHT - y - 1));
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;
      glDrawPixels (WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data()); GL_CHECK_ERRORS;
      glfwSwapBuffers(window);
      sleep(5);
      return 0;
    }
    
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;
    glDrawPixels (WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data()); GL_CHECK_ERRORS;

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;

}

void DrawPrize(int counter, Image &img) {
  int W = 850;
  int H = 730;
  int  i = W + 50, j = H + 12;
  int bias_x, bias_y;
  Image prize("../resources/prize.png");
  for (int x = W, k = 0; x < W + tileSize; x++, k++) {
    for (int y = H, m = 0; y < H + tileSize; y++, m++) {
      if ((prize.GetPixel(k, m)).a != 0)
        img.PutPixel(x, y, prize.GetPixel(k, m));
    }
  }
}

textt change_file(std::string path, Player &player) {
  std::fstream file(path, std::ios::in|std::ios::out);
  char temp;
  textt txt;
  int i = 0, j = 0;
  std::vector<char> str;
  while(true)
  {
      file >> std::noskipws >> temp;
      if (temp != '\n') {
        i++;
      }
      else {
        txt.push_back(str);
        str.clear();
        i = 0;
        j++;
      }
      if(temp == 'G') {
        if (player.getCoords().y / 32 == j && player.getCoords().x / 32 == i - 1) {
          file.seekp(-1, std::ios::cur);
          file << '.';
        }
      }
      str.push_back(temp);
      if( file.eof() ) {
        file.close();
        break;
      }
        
  }
  return txt;   
}

void read_map(std::string path) {
  std::fstream fin(path, std::fstream::in);
  
  char ch;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {

      fin >> ch;

      if( fin.eof() ) return;
      
      all_rooms[i][j] = ch;

    }
  }
  
}


textt read_file(std::string path, Image &img, Hall *room) {
  Image wall("../resources/wall.png");
  Image bar1(room->path);
  Image prize("../resources/prize.png");
  textt txt;
  std::vector<char> str;
  std::fstream fin(path, std::fstream::in);

  int i = 0;
  int j = 0;
  int z = 0, p = 0;
  int tmp_right = 0;
  int tmp_left = 0;
  int tmp_up = 0;
  int tmp_down = 0;

  int bias_x = 2;
  int bias_y = 2;

  while (true) {
    char ch;
    fin >> std::noskipws >> ch;
    str.push_back(ch);
    if (ch != '\n')
      z++;
    else {
      z = 0;
      p++;
    }
    switch (ch) {

      case '#':
        for (int x = i * tileSize, k = 0; x < i * tileSize + tileSize; x++, k++) {
          for (int y = j * tileSize, m = 0; y < j * tileSize + tileSize; y++, m++) {
            img.PutPixel(x, y, wall.GetPixel(k, m));
          }
        }
        break;

      case '@':
        z--;
        starting_pos.x = z * tileSize;
        starting_pos.y = p * tileSize;
        for (int x = i * tileSize, k = 0; x < i * tileSize + tileSize; x++, k++) {
          for (int y = j * tileSize, m = 0; y < j * tileSize + tileSize; y++, m++) {
            img.PutPixel(x, y, bar1.GetPixel(room->tile_bias_x * tileSize + k, room->tile_bias_y  * tileSize + m));
          }
        }
        break;

      case '.':
        for (int x = i * tileSize, k = 0; x < i * tileSize + tileSize; x++, k++) {
          for (int y = j * tileSize, m = 0; y < j * tileSize + tileSize; y++, m++) {
            img.PutPixel(x, y, bar1.GetPixel(room->tile_bias_x * tileSize + k, room->tile_bias_y  * tileSize + m));
          }
        }
        break;

      case ' ':
        break;
        

      case 'G':
        for (int x = i * tileSize, k = 0; x < i * tileSize + tileSize; x++, k++) {
          for (int y = j * tileSize, m = 0; y < j * tileSize + tileSize; y++, m++) {
            if ((prize.GetPixel(k, m)).a != 0)
              img.PutPixel(x, y, prize.GetPixel(k, m));
            else 
              img.PutPixel(x, y, bar1.GetPixel(k, m));
          }
        }
        break;

      case '\n':
          break;


      default:
        for (int x = i * tileSize, k = 0; x < i * tileSize + tileSize; x++, k++) {
          for (int y = j * tileSize, m = 0; y < j * tileSize + tileSize; y++, m++) {
            img.PutPixel(x, y, bar1.GetPixel(room->tile_bias_x * tileSize + k, room->tile_bias_y  * tileSize + m));
          }
        }
        break;

    }

    i++;
    if (ch == '\n') {
      txt.push_back(str);
      str.clear();
      i = 0;
      j++;
    }
     
    if( fin.eof() ) break;

  }




  return txt;
}




std::string get_hall(char ch, Hall *room) {
  room->path = "../resources/floor.png";
  room->tile_bias_x = 0;
  room->tile_bias_y = 0;

  if (ch == 'A') return "../resources/room1.txt";
  else if (ch == 'B') return "../resources/room2.txt";
  else if (ch == 'C') return "../resources/room3.txt";
  else if (ch == 'D') return "../resources/room4.txt";
  else if (ch == 'E') return "../resources/room5.txt";
  else if (ch == 'F') return "../resources/room6.txt";
  else if (ch == 'N') return "../resources/room7.txt";
  else if (ch == 'H') return "../resources/room8.txt";
  else if (ch == 'L') return "../resources/room9.txt";
  else if (ch == 'G') return "../resources/room10.txt";
  else if (ch == 'K') return "../resources/room11.txt";
  else if (ch == 'M') return "../resources/room12.txt";
  else if (ch == 'Z') return "../resources/room13.txt";
  else if (ch == 'P') return "../resources/room14.txt"; 
  return "";
}