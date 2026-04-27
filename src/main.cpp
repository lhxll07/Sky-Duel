#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <random>
#include <list>
#include <algorithm>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// 图片解析函数
unsigned int load_img_to_texture(const char* img_path, int& img_w, int& img_h) {
    //stbi_set_flip_vertically_on_load(true); 关闭自动翻转

    int channels;
    unsigned char* pixel_data = stbi_load(img_path, &img_w, &img_h, &channels, 0);
    if (!pixel_data) {
        printf("图片加载失败：%s\n", img_path);
        printf("stb_image 错误原因：%s\n", stbi_failure_reason());
        return 0;
    }

    unsigned int tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_w, img_h, 0, format, GL_UNSIGNED_BYTE, pixel_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixel_data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex_id;
}

class plane_e{
    private:
    double loc_l;
    double loc_w;
    int speed;
    double last_time = 0;
    public:
    plane_e(double loc_l, double loc_w, int speed): 
        loc_l{loc_l}, loc_w{loc_w}, speed{speed} {}

    vector<int> get_loc(){
        return {static_cast<int>(loc_l), static_cast<int>(loc_w)};
    }
    
    bool move(int length, double current_time){
        if(last_time == 0){
            last_time = current_time;
        }
        
        double delta_time = current_time - last_time;
        last_time = current_time;

        if(loc_l + speed * delta_time < length){
            loc_l += speed * delta_time;
            return true;
        }else{
            return false;
        }
    }
};

class bullet{
    private:
    double loc_l;
    double loc_w;
    int speed;
    int display_times;
    double last_time = 0;
    public:
    bullet(double loc_l, double loc_w, int speed, int display_times): 
        loc_l{loc_l}, loc_w{loc_w}, speed{speed}, display_times(display_times){}

    // 判断是否击中
    list<plane_e>::iterator if_catch(list<plane_e>& planes, int b_length, int b_width){
        for(auto i = planes.begin();i!=planes.end();i++){
            int dl = abs(i->get_loc()[0] - static_cast<int>(loc_l));
            int dw = abs(i->get_loc()[1] - static_cast<int>(loc_w));
            if(dl <= b_length/display_times && dw <= b_width/display_times){
                return i;
            }
        }
        return planes.end();
    }

    vector<int> get_loc(){
        return {static_cast<int>(loc_l), static_cast<int>(loc_w)};
    }

    bool move(double current_time){
        if(last_time == 0){
            last_time = current_time;
        }

        double delta_time = current_time - last_time;
        last_time = current_time;

        if(loc_l >= speed * delta_time){
            loc_l -= speed * delta_time;    
            return true;
        }
        return false;
    }
};

class plane_m{
    private:
    double loc_l;
    double loc_w;
    int speed;
    double last_time_move = 0;
    double last_time_fire = 0;
    double fire_cooldown;
    public:
    plane_m(double loc_l, double loc_w, int speed, double fire_cooldown): 
        loc_l{loc_l}, loc_w{loc_w}, speed{speed}, fire_cooldown{fire_cooldown} {}

    vector<int> get_loc(){
        return {static_cast<int>(loc_l), static_cast<int>(loc_w)};
    }
    bool set_loc(int loc_ll, int loc_ww){
        loc_l = loc_ll;
        loc_w = loc_ww;
        return true;
    }

    bool add_speed(int add){
        speed += add;
        return true;
    }
    
    int get_speed(){
        return speed;
    }

    bool reduce_cooldown(double reduce){
        if(fire_cooldown > reduce + 0.05){
            fire_cooldown -= reduce;
            return true;
        }else{
            return false;
        }
    }

    bool move(int width, double current_time, int dire) {
        if (last_time_move == 0) {
            last_time_move = current_time;
            return false;
        }
        double delta_time = current_time - last_time_move;
        last_time_move = current_time;

        switch(dire) {
            case 1: {   // 向左
                double new_w = loc_w - speed * delta_time;
                loc_w = max(0.0, new_w);
                return true;
            }
            case 2: {   // 向右
                double new_w = loc_w + speed * delta_time;
                loc_w = min(static_cast<double>(width - 1), new_w);
                return true;
            }
            default:
                return false;
        }
    }

    bool fire(list<bullet>& b, int b_length, int b_width, int bullet_speed, int display_times, double current_time){
        if(last_time_fire == 0){
            b.push_back({loc_l - static_cast<int>(b_length / display_times), loc_w, bullet_speed, display_times});
            last_time_fire = current_time;
            return true;
        }
        else if(current_time - last_time_fire >= fire_cooldown){
            b.push_back({loc_l - static_cast<int>(b_length / display_times), loc_w, bullet_speed, display_times});
            last_time_fire = current_time;
            return true;
        }else{
            return false;
        }
    }
};

class board{
    private:
    int is_plm = 1;
    int is_ple = 2;
    int is_blank = 0;
    int is_bullet = 3;
    int b_length;
    int b_width;
    int e_speed;
    int bl_speed;
    int marks;
    int target;
    int health = 200;
    int display_times = 20;
    double last_spawn_time = 0;
    double spawn_time = 1;
    vector<vector<int>> b;
    plane_m planes_m;
    list<plane_e> planes_e;
    list<bullet> bullets;

    // 随机数引擎
    random_device rdm;
    mt19937 engine;
    uniform_int_distribution<int> dist;
    public:

    // 构造界面
    board(int length, int width, int m_speed, int e_speed, int bl_speed, int mark, int target, double spawn_time): 
        b_length{length}, b_width{width}, e_speed{e_speed}, bl_speed{bl_speed}, marks{mark}, target{target}, spawn_time{spawn_time}, 
        b(length, vector<int>(width)), 
        planes_m(length-1,0,m_speed,1), 
        engine(rdm()),
        dist(0,width-1) {}

    void operator=(const board& other){
        b_length = other.b_length;
        b_width = other.b_width;
        e_speed = other.e_speed;
        bl_speed = other.bl_speed;
        marks = other.marks;
        target = other.target;
        health = other.health;
        display_times = other.display_times;
        last_spawn_time = other.last_spawn_time;
        spawn_time = other.spawn_time;
        b = other.b;
        planes_m = other.planes_m;
        planes_e = other.planes_e;
        bullets = other.bullets;
        engine = other.engine;
        dist = other.dist;
    }

    // 传值
    int get_mark(){ return marks; }
    int get_target(){ return target; }
    int get_m_speed(){ return planes_m.get_speed(); }
    int get_e_speed(){ return e_speed; }
    double get_spawn_speed(){ return spawn_time; }

    // 发射
    void fire(double current_time){
        planes_m.fire(bullets, b_length, b_width, bl_speed, display_times, current_time);
    }

    // 撞击时减少血量
    bool change_health(){
        if(health > 20){
            health -= 20;
            return true;
        }else{
            return false;
        }
    }

    // 移动己方战机
    void move(int dire, double current_time){
        planes_m.move(b_width,current_time,dire);
    }

    // 随机生成敌机(刷新生成时间)
    void random_init(double current_time){
        if(last_spawn_time == 0){
            last_spawn_time = current_time;
            return;
        }
        double delta_time = current_time - last_spawn_time;
        if(delta_time >= spawn_time){
            last_spawn_time = current_time;
            if(spawn_time > 0.1){
                spawn_time -= 0.02;
            }
            int tar = dist(engine);
            planes_e.push_back({0.0, static_cast<double>(tar), e_speed});
        }
    }

    // 刷新每一帧
    int flash(double current_time){
        // 移动敌机并判定是否撞击
        if(target <= marks){
            return 2;
        }
        random_init(current_time);
        for(auto x = planes_e.begin();x!=planes_e.end();){
            int dl = abs(x->get_loc()[0] - planes_m.get_loc()[0]);
            int dw = abs(x->get_loc()[1] - planes_m.get_loc()[1]);
            if(dl <= b_length / display_times * 2 && dw <= b_width / display_times){
                x = planes_e.erase(x);
                if(!change_health()){
                    return 0;
                }
            }else if(x->move(b_length, current_time)){
                x++;
            }else{
                x = planes_e.erase(x);
            }
        }
        // 移动并判定子弹
        for(auto x = bullets.begin();x!=bullets.end();){
            if(x->move(current_time)){
                auto temp = x->if_catch(planes_e, b_length, b_width);
                if(temp != planes_e.end()){
                    marks += 10;
                    if(marks % 100 == 0){
                        planes_m.add_speed(4);
                        planes_m.reduce_cooldown(0.1);
                        
                    }
                    planes_e.erase(temp);
                    x = bullets.erase(x);
                }else{
                    x++;
                }
            }else{
                x = bullets.erase(x);
            }
        }
        // 刷新界面
        for(int y = 0 ; y < b_length;y++){
            for(int x = 0;x < b_width;x++){
                b[y][x] = 0;
            }
        }
        b[planes_m.get_loc()[0]][planes_m.get_loc()[1]] = is_plm;
        for(auto& x : planes_e){
            b[x.get_loc()[0]][x.get_loc()[1]] = is_ple;
        }
        for(auto& x : bullets){
            b[x.get_loc()[0]][x.get_loc()[1]] = is_bullet;
        }
        return 1;
    }

    // 显示每一帧
    void display(unsigned my_plane, unsigned enemy_plane, unsigned bullet,
                 int display_w, int display_h){
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); 
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // 显示当前得分、生命值、目标得分
        string Score = "Score:" + to_string(marks);
        string Health = "Health:" + to_string(health);
        string Target = "Target:" + to_string(target);
        draw_list->AddText(NULL,40,ImVec2(0,0),IM_COL32(255, 255, 255, 255),Score.c_str());
        draw_list->AddText(NULL,40,ImVec2(0,40),IM_COL32(255, 255, 255, 255),Target.c_str());
        draw_list->AddText(NULL,40,ImVec2(display_w - 240,0),IM_COL32(255, 255, 255, 255),Health.c_str());

        // 显示己方与地方战机、子弹
        for(auto y = 0;y < b_length;y++){
            for(auto x = 0;x < b_width;x++){
                auto n = b[y][x];
                int times_w = display_w / b_width;
                int times_h = display_h / b_length;
                int times = (times_w + times_h) * display_times / 7;
                if(n == is_blank){
                    continue;
                }else if(n == is_ple){
                    ImVec2 img_min(x*times_w, y*times_h);
                    ImVec2 img_max(x*times_w+times, y*times_h+times);
                    draw_list->AddImage(enemy_plane, img_min, img_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
                }else if(n == is_plm){
                    ImVec2 img_min(x*times_w, display_h-times);
                    ImVec2 img_max(x*times_w+times, display_h);
                    draw_list->AddImage(my_plane, img_min, img_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
                }else if(n == is_bullet){
                    ImVec2 img_min(x*times_w, y*times_h);
                    ImVec2 img_max(x*times_w+times, y*times_h+times);
                    draw_list->AddImage(bullet, img_min, img_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
                }
            }
        }
        ImGui::PopStyleVar(2);
    }
};

// 开始界面
void Start(int& flag, GLFWwindow* window){
    if(flag == 1){
        return;
    }else{
        const char* btns[] = {"Start(Single Player)","Quit"};
        float region_width = ImGui::GetContentRegionAvail().x;
        float region_length = ImGui::GetContentRegionAvail().y;
        float single_width = 300;
        float single_length = 150;
        int btns_num = sizeof(btns) / sizeof(btns[0]);
        for(auto i = 0;i < btns_num;i++){
            ImGui::NewLine();
            ImGui::SetCursorPosX((region_width - single_width) / 2);
            ImGui::SetCursorPosY((region_length - single_length * 3 * (1-i)) / 2);
            if(ImGui::Button(btns[i], ImVec2(single_width,single_length))){
                switch(i){
                    case 0: flag = 1; break;
                    case 1: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
                }
            }
        }
    }
}

// 失败界面
void Failed(board& b, int& flag, int display_h, int display_w, GLFWwindow* window){
    if(flag == 0){
        return;
    }else{
        const char* btns[] = {"Restart","Quit"};
        float region_width = ImGui::GetContentRegionAvail().x;
        float region_length = ImGui::GetContentRegionAvail().y;
        float single_width = 300;
        float single_length = 150;
        int btns_num = sizeof(btns) / sizeof(btns[0]);
        for(auto i = 0;i < btns_num;i++){
            ImGui::NewLine();
            ImGui::SetCursorPosX((region_width - single_width) / 2);
            ImGui::SetCursorPosY((region_length - single_length * 3 * (1-i)) / 2);
            if(ImGui::Button(btns[i], ImVec2(single_width,single_length))){
                switch(i) {
                    case 0: {
                        board a {100,100,100,70,100,0,200,2};
                        b = a;
                        flag = 0;
                    } break;
                    case 1: {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    } break;
                }
            }
        }
    }
}
// 下一关界面
void Next(board& b, int& flag, int display_h, int display_w, GLFWwindow* window){
    if(flag == 0){
        return;
    }else{
        const char* btns[] = {"Next","Quit"};
        float region_width = ImGui::GetContentRegionAvail().x;
        float region_length = ImGui::GetContentRegionAvail().y;
        float single_width = 300;
        float single_length = 150;
        int btns_num = sizeof(btns) / sizeof(btns[0]);
        for(auto i = 0;i < btns_num;i++){
            ImGui::NewLine();
            ImGui::SetCursorPosX((region_width - single_width) / 2);
            ImGui::SetCursorPosY((region_length - single_length * 3 * (1-i)) / 2);
            if(ImGui::Button(btns[i],ImVec2(single_width,single_length))){
                switch(i) {
                    case 0: {
                        board a {100,100,b.get_m_speed(),70,100,b.get_mark(),b.get_target() * 2, b.get_spawn_speed()};
                        b = a;
                        flag = 0; 
                    }break;     // 下一关
                    case 1: glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
            }
        }
    }
}


int main(){
    if(!glfwInit()){
        printf("GLFW 初始化失败！\n");
        return -1;
    }
    // 初始化GLFW版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // 初始化GLFW窗口
    glfwWindowHint(GLFW_VISIBLE, true);
    GLFWwindow* window = glfwCreateWindow(1050, 1000, "Sky Duel", NULL, NULL);
    if(!window){
        printf("窗口创建失败！\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);    // 打开垂直同步

    // GLAD初始化
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("GLAD 初始化失败！\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // 设置平台，ImGui后端(OpenGL窗口)
    ImGui_ImplGlfw_InitForOpenGL(window, true);        // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init("#version 330");

    // (Your code calls glfwPollEvents())
    int img_w_myp, img_h_myp, img_w_ep, img_h_ep, img_h_bl, img_w_bl;
    unsigned int myp_tex_id = load_img_to_texture("./pictures/myplane.png", img_w_myp, img_h_myp);
    unsigned int ep_tex_id = load_img_to_texture("./pictures/enemyplane.png", img_w_ep, img_h_ep);
    unsigned int bullet_tex_id = load_img_to_texture("./pictures/bullet.png", img_w_bl, img_h_bl);
    if(myp_tex_id == 0 || ep_tex_id == 0 || bullet_tex_id == 0){
        printf("资源加载失败，请确认 pictures 文件夹在程序工作目录下。\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    int is_started_flag = 0;
    int is_game_over_flag = 0;
    int is_game_next_flag = 0;
    board b{100,100,100,70,100,0,200,2};

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        if(is_started_flag && !is_game_over_flag && !is_game_next_flag){
            double current_time = glfwGetTime();
            bool move_left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                             glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
            bool move_right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
                              glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
            bool fire_pressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                                glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;

            // 按键检测（左右移动）
            if(move_left){
                b.move(1,current_time);
            }else if(move_right) {
                b.move(2,current_time);
            }
            b.move(3, current_time);// 刷新移动时间

            if(fire_pressed) {
                b.fire(current_time);
            }else if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                is_started_flag = 0;
            }

            switch(b.flash(current_time)){
                case 1: break;
                case 0: is_game_over_flag = 1;break;
                case 2: is_game_next_flag = 1;break;
            }
        }

        // 开始新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        glClear(GL_COLOR_BUFFER_BIT);
        
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h); // 获取 GLFW 窗口绘制区大小
        glViewport(0, 0, display_w, display_h);
        ImGui::SetNextWindowPos(ImVec2(0, 0)); // 位置：GLFW 窗口左上角
        ImGui::SetNextWindowSize(ImVec2(display_w, display_h)); // 大小：和 GLFW 窗口一致
        ImGui::Begin("Sky Duel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        // 游戏判断逻辑
        if(is_started_flag && !is_game_over_flag && !is_game_next_flag){
            b.display(myp_tex_id, ep_tex_id, bullet_tex_id, display_w, display_h);
        }else if(!is_started_flag){
            Start(is_started_flag, window);
        }else if(is_game_over_flag){
            Failed(b, is_game_over_flag, display_h, display_w, window);
        }else if(is_game_next_flag){
            Next(b, is_game_next_flag, display_h, display_w, window);
        }
        
        ImGui::End(); // 关闭窗口

        // Rendering

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    
    // 清理资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
