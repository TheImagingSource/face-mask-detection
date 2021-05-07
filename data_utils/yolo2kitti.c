/* LICENSE.MIT */

/* A small converter of labels from YOLO format to KITTI format 

YOLO format example: 
16 0.292187 0.433333 0.228125 0.333333

The format of the label file is

LABEL_ID_1 X_CENTER_NORM Y_CENTER_NORM WIDTH_NORM HEIGHT_NORM
LABEL_ID_2 X_CENTER_NORM Y_CENTER_NORM WIDTH_NORM HEIGHT_NORM

The LABEL_ID_# is the index number in the classes.txt file. The id of the first label is 0.
Please note that this program is able to convert label files that have only one label id (one line). You will need to extend this program if you have label files with attributes of multiple bounding boxes.

Position attributes in the label file are not absolute but normalised:
X_CENTER_NORM = X_CENTER_ABS/IMAGE_WIDTH
Y_CENTER_NORM = Y_CENTER_ABS/IMAGE_HEIGHT
WIDTH_NORM = WIDTH_OF_LABEL_ABS/IMAGE_WIDTH
HEIGHT_NORM = HEIGHT_OF_LABEL_ABS/IMAGE_HEIGHT

KITTI format after convertion in this case is:
car 0.00 0 0.00 57.00 64.00 130.00 144.00 0.00 0.00 0.00 0.00 0.00 0.00 0.00
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

/* Change these inputs according to your dataset */
#define LABEL_PATH_INPUT "./Faces/mask/labels_YOLO_mask"
#define LABEL_PATH_OUTPUT "./Faces/mask/labels"
#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240
#define LABEL "mask"

/* Write attributes in KITTI format to a file */
void write_file(float* ptr, char** filename)
{
    FILE* file;
    char* fullFileName = malloc(15000);
    int arrayLength;
    arrayLength = sizeof(*ptr);

    // Merge full path and a file name
    sprintf (fullFileName, "%s/%s", LABEL_PATH_OUTPUT, *filename);
    file = fopen(fullFileName, "w");

    // Add label and 3 first coordinates (not used in TLT, thus always 0)
    fprintf(file, "%s 0.00 0 0.00 ", LABEL);

    // Add actual bbox coordinates
    for (int i = 0; i < arrayLength; i++)
    {
        fprintf(file, "%.2f ", *ptr);
        *ptr++;
    }

    // Add last 7 coordinates (not used in TLT, thus always 0)
    fprintf(file, "0.00 0.00 0.00 0.00 0.00 0.00 0.00");
    fclose(file);
}

/* Convert bbox coordinates from YOLO format to KITTI format */
float convert_bbox(float* bbox)
{
    int arrayLength;
    float dx, dy, xmin, ymin, xmax, ymax;;

    bbox[0] *= IMAGE_WIDTH;
    bbox[1] *= IMAGE_HEIGHT;
    bbox[2] *= IMAGE_WIDTH;
    bbox[3] *= IMAGE_HEIGHT;
    dx = fabs(0.5 * bbox[2]);
    dy = fabs(0.5 * bbox[3]);
    xmin = bbox[0] - dx;
    ymin = bbox[1] - dy;
    xmax = bbox[0] + dx;
    ymax = bbox[1] + dy;
    bbox[0] = xmin;
    bbox[1] = ymin;
    bbox[2] = xmax;
    bbox[3] = ymax;

    return xmin, ymin, xmax, ymax;
}

/*  Read label file, store attributes and convert them */
void read_file(float** ptr, char** fileName)
{
    float* c = calloc(50, sizeof(float));
    int* a = calloc(50, sizeof(int));
    int i = 0;
    float xmin, ymin, xmax, ymax;
    FILE* file;
    char* fullFileName = malloc(15000);
    sprintf (fullFileName, "%s/%s", LABEL_PATH_INPUT, *fileName);
 
    file = fopen(fullFileName, "r");
    if (file)
    {
        while ((c[i] = getc(file)) != EOF)
        {
            fscanf(file, "%d %f", &a[i], &c[i]);
            i++;
        }
        *ptr = c;
        fclose(file);
    }
    xmin, ymin, xmax, ymax = convert_bbox(*ptr);
    write_file(*ptr, fileName);
}

void main()
{
    int arrayLength;
    float dx, dy, xmin, ymin, xmax, ymax;
    float* bbox;
    size_t i = 0;
    char** fileNames = malloc(5000);
    DIR *d;
    struct dirent *dir;
    d = opendir(LABEL_PATH_INPUT);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            fileNames[i] = dir->d_name;
            printf("Initial name: %s\n", dir->d_name);
            if (*fileNames[i] != '.')
            {
                read_file(&bbox, &fileNames[i]);
            }
            i++;
        }
    closedir(d);
    }
    arrayLength = sizeof(*fileNames);
    printf("Number of files: %d\n", arrayLength);
    free(bbox);
}