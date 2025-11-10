#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <chrono>
#include<vector>
#include<atomic>
#include<thread>	
#include<pthread.h>
using namespace std::chrono;
using namespace std;
int bb = 2;
float alpha=1;   // scaling factor

// class ThreadAndLock{

// 	private:
// 		int image_dimension;
// 	public:
// 		vector<int> buffer;



// 		ThreadAndLock(int image_dimension)
// 		{
// 			this->image_dimension=image_dimension;
// 			for(int i=0;i<image_dimension; ++i)
// 			{
// 				buffer.push_back(0);
// 			}
// 		}

// 		void setValue(int index, int value)
// 		{
// 			buffer[index]=value;
// 		}

// };


struct image_t* input_image;
int height,width;
// struct image_t* smoothened_image;
// struct image_t* details_image;
struct image_t* sharpened_image;


struct params{

	int row;
	int col;
};



vector<vector<uint8_t>>buffer_s1_s2;
vector<vector<uint8_t>>buffer_s2_s3;
bool s1_s2=false;
bool s2_s3=false;
std::atomic_flag s1_s2_lock = ATOMIC_FLAG_INIT;
std::atomic_flag s2_s3_lock = ATOMIC_FLAG_INIT;








void S1_smoothen(vector<vector<uint8_t>>& buffer_s1_s2, int row, int col=0)
{
	

	


		int i=row;
	
	
		for(int j=0;j<width;j++)
		{
			int r = 0, g = 0, b = 0;
			for(int y = -bb;y<=bb;y++)
			{
				for(int x=-bb;x<=bb;x++)
				{
					if(i+y < 0)
						continue;
					if(i+y >=height)
						continue;
					if(j+x < 0)
						continue;
					if(j+x >= width)
						continue;
					r+=input_image->image_pixels[i+y][j+x][0];
					g+=input_image->image_pixels[i+y][j+x][1];
					b+=input_image->image_pixels[i+y][j+x][2];

				}
			}
			buffer_s1_s2[j][0] = r/((2*bb + 1)*(2*bb + 1));
			buffer_s1_s2[j][1] = g/((2*bb + 1)*(2*bb + 1));
			buffer_s1_s2[j][2] = b/((2*bb + 1)*(2*bb + 1));
		}
	
	

}

void S2_find_details(vector<vector<uint8_t>>& buffer_s1_s2, vector<vector<uint8_t>>& buffer_s2_s3, int row, int col=0)
{
	
	
	
	
	 for(int j=0;j<width;j++)
	 {
	  for(int k=0;k<3;k++)
	  {
	   int find_details_value = input_image->image_pixels[row][j][k] - buffer_s1_s2[j][k];
	   if(find_details_value > 0)
	   	buffer_s2_s3[j][k] = find_details_value;
	   else
		buffer_s2_s3[j][k] = 0;
	  } 
	 }
	

}

void S3_sharpen(vector<vector<uint8_t>>& buffer_s2_s3, int row, int col=0)
{
	// TODO
	

	
	
	 for(int j=0;j<width;j++)
	 {
	  for(int k=0;k<3;k++)
	  {
	   int sharpen_value = input_image->image_pixels[row][j][k] + static_cast<int>(alpha * buffer_s2_s3[j][k]);
	   if(sharpen_value > 255)
		sharpened_image->image_pixels[row][j][k] = 255;
	   else
		sharpened_image->image_pixels[row][j][k] = sharpen_value;
	  } 
	 }
	

}

void* S1_thread(void* arg)
{
	

	// for(int count=1;count<=1000;++count)
	// {

		cout<<"S1 :"<<endl;

		for(int i=0;i<height;++i)
		{


		
		while(s1_s2_lock.test_and_set()) {
			std::this_thread::yield();
		}
		if(s1_s2==false)
		{
				S1_smoothen(buffer_s1_s2,i);
				s1_s2=true;

		}

		else{
			i--;
			// std::this_thread::yield();
		}


		s1_s2_lock.clear();



		}

		
	// }
}

void* S2_thread(void* arg)
{
	// for(int count=1;count<=1000;++count)
	// {
			cout<<"S2 :"<<endl;
		for(int i=0;i<height;++i)
		{
			while(s2_s3_lock.test_and_set()) {
				std::this_thread::yield();
			}

			if(s2_s3==false && s1_s2== true)
			{
				S2_find_details(buffer_s1_s2,buffer_s2_s3,i);
				s2_s3=true;
				s1_s2=false;

			}

			else{
				i--;
				// std::this_thread::yield();
			}

			s2_s3_lock.clear();
		}
	// }
}


void* S3_thread(void* arg)
{
	// for(int count=1;count<=1000;++count)
	// {
			cout<<"S3 :"<<endl;
		for(int i=0;i<height;++i)
		{
			while(s2_s3_lock.test_and_set()) {
				std::this_thread::yield();
			}
			
				if(s2_s3==true)
				{
					S3_sharpen(buffer_s2_s3,i);
					s2_s3=false;
				}
				else{
					i--;
					// std::this_thread::yield();
				}

				s2_s3_lock.clear();
		}
	// }

}


void allocate_space(struct image_t*& img)
{
	img = new struct image_t;
	img->width = width;
	img->height = height;
	img->image_pixels = new uint8_t**[img->height];
	for(int i = 0; i < img->height; i++)
	{
		img->image_pixels[i] = new uint8_t*[img->width];
		for(int j = 0; j < img->width; j++)
			img->image_pixels[i][j] = new uint8_t[3];
	}



  img->width = input_image->width;
  img->height = input_image->height;

}


void deallocate_space(struct image_t*& img)
{
    if (!img) return; 

    for (int i = 0; i < img->height; i++)
    {
        for (int j = 0; j < img->width; j++)
        {
    
            delete[] img->image_pixels[i][j];
        }
       
        delete[] img->image_pixels[i];
    }

    delete[] img->image_pixels;


    delete img;
    img = nullptr; 
}


int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
        exit(0);
    }

	

    auto start = high_resolution_clock::now();

  	input_image = read_ppm_file(argv[1]);
	 height = input_image->height;
	 width = input_image->width;
	 allocate_space(sharpened_image);
 	 buffer_s1_s2.resize(width, vector<uint8_t>(3, 0));
	 buffer_s2_s3.resize(width, vector<uint8_t>(3, 0));

	// ThreadAndLock* s1_s2 = new ThreadAndLock(width);
	// ThreadAndLock* s2_s3 = new ThreadAndLock(width);

	

	
	pthread_t s1,s2,s3;
	for(int ite =1; ite<=1000;++ite)
	{
		cout<<"Iteration: "<<ite<<endl;
	pthread_create(&s1,NULL,S1_thread,NULL);
	pthread_create(&s2,NULL,S2_thread,NULL);
	pthread_create(&s3,NULL,S3_thread,NULL);
	pthread_join(s1,NULL);
	pthread_join(s2,NULL);
	pthread_join(s3,NULL);
	}

	write_ppm_file(argv[2],sharpened_image);

	deallocate_space(input_image);
    deallocate_space(sharpened_image);

	auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Total time taken: " << duration.count() << " milliseconds" << endl;




    // auto rt = high_resolution_clock::now();
    // auto rtduration = duration_cast<microseconds>(rt - start);
    // cout << "Time taken to read image: " << rtduration.count() << " microseconds" << endl;
    
    // struct image_t *smoothened_image = S1_smoothen(input_image);
    // auto sm = high_resolution_clock::now();
    // auto smduration = duration_cast<microseconds>(sm - rt);
    // cout << "Time taken to smoothen image: " << smduration.count() << " microseconds" << endl;
    
    // struct image_t *details_image = S2_find_details(input_image, smoothened_image);
    // auto dt = high_resolution_clock::now();
    // auto dtduration = duration_cast<microseconds>(dt - sm);
    // cout << "Time taken to find details: " << dtduration.count() << " microseconds" << endl;
    
    // struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
    // auto sh = high_resolution_clock::now();
    // auto shduration = duration_cast<microseconds>(sh - dt);
    // cout << "Time taken to sharpen image: " << shduration.count() << " microseconds" << endl;
    
    // write_ppm_file(argv[2], sharpened_image);
    // auto wr = high_resolution_clock::now();
    // auto wrduration = duration_cast<microseconds>(wr - sh);
    // cout << "Time taken to write image: " << wrduration.count() << " microseconds" << endl;
    
    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<microseconds>(stop - start);
    // cout << "Total time taken: " << duration.count() << " microseconds" << endl;

    return 0;
}