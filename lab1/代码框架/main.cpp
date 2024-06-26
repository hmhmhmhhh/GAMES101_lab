#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle) {
    Eigen::Matrix3f rotation1, I, N;
    I << 1,0,0,
        0,1,0,
        0,0,1;
    N << 0, -axis[2], axis[1],
        axis[2], 0, -axis[0],
        -axis[1], axis[0], 0;

    rotation1 = std::cos(angle / 180 * MY_PI) * I + (1 - std::cos(angle / 180 * MY_PI)) * axis * axis.transpose() + std::sin(angle / 180 * MY_PI) * N;

    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();
    rotation.block(0, 0, 3, 3) = rotation1; //矩阵块操作，0-3横竖赋值为3x3计算出的矩阵

    return rotation;
}


Eigen::Matrix4f get_model_matrix(float rotation_angle)
{ 
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity(); //初始化为I矩阵
    /*
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    Eigen::Matrix4f rotation;
    //cos sin tan使用的弧度值
    rotation << std::cos(rotation_angle / 360 * 2 * MY_PI), std::sin(-rotation_angle / 360 * 2 * MY_PI), 0, 0, 
        std::sin(rotation_angle / 360 * 2 * MY_PI), std::cos(rotation_angle / 360 * 2 * MY_PI), 0, 0,
        0, 0, 1, 0, 0, 0, 0, 1;
*/
    Eigen::Matrix4f rotation = get_rotation(Eigen::Vector3f(0,0,1),  rotation_angle);
    model = rotation * model;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    float n, f, l, r, b, t;
    n = zNear; //****!!
    f = zFar;
    t = -n * std::tan(eye_fov/(2 * 180) * MY_PI);
    b = -t;
    r = aspect_ratio * t;
    l = -r;
    Eigen::Matrix4f ortho_scale, persp_to_ortho, ortho_trans;
    persp_to_ortho << n, 0, 0, 0, 
                        0, n, 0, 0, 
                        0, 0, n + f, -n * f, 
                        0, 0, 1, 0;
    //缩放
    ortho_scale << 2 / (r - l), 0, 0, 0,
            0, 2 / (t - b), 0, 0,
            0, 0, 2 / (n - f), 0,
            0, 0, 0, 1;
    //平移
    ortho_trans << 1, 0, 0, -(l + r) / 2, 
                    0, 1, 0, -(t + b) / 2,
                    0, 0, 1, -(n + f) / 2, 
                    0, 0, 0, 1;
    
    projection = ortho_scale * ortho_trans * persp_to_ortho * projection;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
