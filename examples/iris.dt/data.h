

typedef enum iris_class_t
{
  iris_unknown=-1,
  iris_setosa=0,
  iris_versicolor=1,
  iris_virginica=2
} iris_class_t;


const char* iris_name(iris_class_t class);
iris_class_t iris_class(const char* name);

typedef struct data_record_t
{
  uint32_t attrs[4];
  iris_class_t class;
} data_record_t;

typedef struct data_t
{
  size_t size;
  data_record_t* data;
} data_t;

data_t* data_init(const char* filename);
void data_free(data_t*);
int data_dump(const data_t* d,const char* fn);

