
#include <sqlite3.h>
#include <iostream>
#include <regex>


//stringstream
#include <iomanip>
using namespace std;

//strlen
#include <cstring>

//結果の行数だけ呼ばれる関数。
int callback(void *anything, int keyCount, char **value, char **key){

    *(int*)anything = *(int*)anything + 1;
    int res =0;

    for(int i=0; i<keyCount; i++){
        //cout << "key   :" << key[i]   << endl;
        //cout << "value :" << value[i] << endl;
        cout << key[i] << " : " << value[i] << "   " ;
    }
    printf( "\n" );

    return 0;
}


int main(int argc, char* argv[]) {
    sqlite3 *db = NULL;
    sqlite3_open( "./test.db", &db );
    char* err = NULL;



    int res = sqlite3_exec( db , "create table TestTable( id int , word text , X text default 'X' );" , NULL , NULL , &err );
    res = sqlite3_exec( db , "insert into TestTable( id , word ) values( 1 , 'Do' );" , NULL , NULL , &err );
    if( res != 0 ){
        printf( "%s\n" , err );
    }

    int count = 0;
    int x = sqlite3_exec( db , "select * from TestTable" , callback , (void*)&count , &err );

    printf( "%d\n" ,count );
    printf( "%d\n" ,res );
    printf( "%s\n" , err );

    //res = sqlite3_exec( db , "insert into TestTable values( 1 , 'Do' );" , NULL , NULL , &err );
    //res = sqlite3_exec( db , "select * from TestTable" , callback , NULL , &err );
    //res = sqlite3_exec( db , "delete from TestTable where id = 99 and word = 'Do' " , NULL , NULL , &err );
    //res = sqlite3_exec( db , "update TestTable set id = 2 where id = 1 and word = 'Do' " , callback , NULL , &err );



    sqlite3_close( db );

    return 0;
}




























