#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <regex>

//sqlite
#include <sqlite3.h>

//stringstream
#include <iomanip>

using namespace std;

static stringstream s;
static int newMailCount = 0;

typedef void* HGLOBAL;
char* resBuf;

FILE* ConsoleWindow;

char* pluginDirectory;
wchar_t* dbPATH;
sqlite3 *db = NULL;

static bool bootOne = false;

int main(int argc, char* argv[]) {
    printf( "%s\n" , argv[0] );
    return 0;
}

#define Debug



//hにはdllまでのLogFilePathが入っている。
//lenはアドレスの長さ。\0の分は入っていない。
extern "C" __declspec(dllexport) bool __cdecl load(HGLOBAL h, long len){

#ifdef Debug
    AllocConsole();
    //標準出力(stdout)を新しいコンソールに向ける
    freopen_s(&ConsoleWindow, "CONOUT$", "w+", stdout);
    //標準エラー出力(stderr)を新しいコンソールに向ける
    freopen_s(&ConsoleWindow, "CONOUT$", "w+", stderr);
    //文字コードをutf-8に変更。
    system( "chcp 65001 > /nul" );
#endif

    pluginDirectory = (char*)malloc(sizeof(char) * (len + 1 ));
    memset( pluginDirectory , '\0' , ( len + 1 ));
    memcpy( pluginDirectory , (char*)h , (int)len  );
    GlobalFree( h );


    int dbpathlen = strlen( pluginDirectory ) + strlen( "mailBox.db" ) + 1;
    char* charDBpath = (char*)calloc( dbpathlen , sizeof(char) );
    sprintf( charDBpath , "%smailBox.db" , pluginDirectory );

    //パスの日本語対応化
    char* oldLocale = setlocale( LC_ALL , NULL );
    setlocale( LC_ALL , "" );
    dbPATH = (wchar_t*)calloc( len + 1 + strlen( "mailBox.db" ) , sizeof( wchar_t ) );
    mbstowcs(dbPATH , charDBpath, len + 1 + strlen( "mailBox.db" ));
    setlocale( LC_ALL , oldLocale );

    free( charDBpath );


    char* err = NULL;
    //一部のユーザはここで失敗していると思う。
    //dbパスに日本語が含まれているとかどうだ。
    int res = sqlite3_open16( dbPATH , &db );
    //int res = sqlite3_open16( dbPATH , &db );
    int sqliteRes = sqlite3_exec( db , "create table mailBox( GhostMenuName text , MailID int , yyyymmdd int , Sender text , Title text , MailText text , Checked int );" , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif

    sqlite3_close( db );

    return true;
}

extern "C" __declspec(dllexport) bool __cdecl unload(void){
    free( pluginDirectory );

#ifdef Debug
    FreeConsole();
#endif

    pluginDirectory = NULL;
    dbPATH          = NULL;
    db              = NULL;

    return true;
}



//|               |        |          |        |       |          |         |
//| GhostMenuName | MailID | YYYYmmdd | Sender | Title | MailText | Checked |
//|               |        |          |        |       |          |         |
int callbackMailList(void *anything, int keyCount, char **value, char **key){
    
    //printf( "%d\n" , *(int*)anything );
    s << "├┼\\_a[OnOpenMail," << value[0] << "," << value[1] << "," << *(int*)anything << "]" << value[3] << " : " << value[4] << "\\_a\\n";
    return 0;
}
int callbackOpenMail(void *anything, int keyCount, char **value, char **key){
    s << "   【" << value[4] << "】" << "\\n\\n" << value[5] << "\\n\\n\\n   【" << value[0] << "】";
    return 0;
}
int callbackNewMailCount(void *anything, int keyCount, char **value, char **key){
    newMailCount++;
    return 0;
}
int callbackStatusMail(void *anything, int keyCount, char **value, char **key){
    string Checked      = value[6];
    string nowYYYYmmdd  = value[2];

    string strYMD;
    stringstream streamYMD;
    time_t t = time(nullptr);
    const tm* localTime = localtime(&t);
    streamYMD << localTime->tm_year + 1900;
    streamYMD << setw(2) << setfill('0') << localTime->tm_mon + 1;
    streamYMD << setw(2) << setfill('0') << localTime->tm_mday;
    streamYMD >> strYMD;

    int now     = atoi( strYMD.c_str() );
    int mailDay = atoi( nowYYYYmmdd.c_str() );
    //届いてない
    if ( mailDay > now ){
        *(int*)anything = 1;
    } else {
        //届いていて未読
        if ( Checked == "0" ){
            *(int*)anything = 2;
        //届いていて既読
        } else {
            *(int*)anything = 3;
        }
    }
    streamYMD.str("");
    streamYMD.clear( stringstream::goodbit );
    return 0;
}


//|               |        |          |        |       |          |         |
//| GhostMenuName | MailID | YYYYmmdd | Sender | Title | MailText | Checked |
//|               |        |          |        |       |          |         |
void SendMail( char* GhostMenuName , char* MailID , char* YYYY , char* MM , char* DD , char* Sender , char* Title , char* MailText ){
    //引数が正しくなくnullの可能性。足りない場合の処理
    char* err = NULL;

    string strGhostMenuName  = GhostMenuName;
    strGhostMenuName         = regex_replace( GhostMenuName , regex( "'" ) ,"\"" );

    string strMailID         = MailID       ;

    //二桁にすること。結合すること。
    string strYYYY           = YYYY         ;
    string strMM             = MM           ;
    string strDD             = DD           ;
    //0X月0Y日
    stringstream streamMM;
    stringstream streamDD;
    streamMM << setw( 2 ) << setfill( '0' ) << strMM    ;
    streamDD << setw( 2 ) << setfill( '0' ) << strDD    ;

    string strYYYYmmdd = strYYYY + streamMM.str() + streamDD.str();
    ////サニタイズチェック
    //数値のチェック
    string strCheck;
    strCheck = regex_replace( strYYYYmmdd , regex( R"([0-9])" ) ,"" );
    if( strCheck != "" ){ return; }
    strCheck = regex_replace( strMailID , regex( R"([0-9])" ) ,"" );
    if( strCheck != "" ){ return; }

    string strSender        = Sender       ;
    strSender               = regex_replace( strSender , regex( "'" ) ,"\"" );
    strSender               = regex_replace( strSender , regex( "vanish" ) ,"危険な文字" );
    string strTitle         = Title        ;
    strTitle                = regex_replace( strTitle , regex( "'" ) ,"\"" );
    strTitle                = regex_replace( strTitle , regex( "vanish" ) ,"危険な文字" );
    string strMailText      = MailText     ;
    strMailText             = regex_replace( strMailText , regex( "'" ) ,"\"" );
    strMailText             = regex_replace( strMailText , regex( "vanish" ) ,"危険な文字" );




    sqlite3_open16( dbPATH , &db );
    int sqliteRes = sqlite3_exec( db , "create table mailBox( GhostMenuName text , MailID int , yyyymmdd int , Sender text , Title text , MailText text , Checked int );" , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif
    //int createTable = sqlite3_exec( db , "create table mailBox( GhostMenuName text , MailID int , yyyymmdd int , Sender text , Title text , MailText text , Checked int );" , NULL , NULL , &err );
    //if ( createTable == 1 ) {
    //    printf( "%s\n" , err );
    //}


    string sqlDelete = "delete from mailBox where GhostMenuName ='" + strGhostMenuName + "' and MailID = '" + strMailID + "'";
    sqliteRes = sqlite3_exec( db , sqlDelete.c_str() , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif

    string sqlInsert = "insert into mailBox values( '" + strGhostMenuName + "' , " + strMailID + " , " + strYYYYmmdd + " , '" + strSender + "' , '" + strTitle + "' , '" + strMailText + "' , 0)"; 
    sqliteRes = sqlite3_exec( db , sqlInsert.c_str() , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif


    sqlite3_close( db );
    streamMM.str("");
    streamMM.clear( stringstream::goodbit );
    streamDD.str("");
    streamDD.clear( stringstream::goodbit );
}
void DeleteMail( char* GhostMenuName , char* MailID ){
    char* err = NULL;

    string strGhostMenuName  = GhostMenuName;
    strGhostMenuName         = regex_replace( GhostMenuName , regex( "'" ) ,"\"" );

    string strMailID         = MailID       ;

    string strCheck;
    strCheck = regex_replace( strMailID , regex( R"([0-9])" ) ,"" );
    if( strCheck != "" ){ return; }

    sqlite3_open16( dbPATH , &db );
    string sqlDelete = "delete from mailBox where GhostMenuName ='" + strGhostMenuName + "' and MailID = '" + strMailID + "'";
    int sqliteRes = sqlite3_exec( db , sqlDelete.c_str() , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif
    sqlite3_close( db );
}
//err -1
//0メールが存在しない
//1まだ届いていない
//2未読
//3既読
int StatusMail( char* GhostMenuName , char* MailID ){
    char* err = NULL;

    string strGhostMenuName  = GhostMenuName;
    strGhostMenuName         = regex_replace( GhostMenuName , regex( "'" ) ,"\"" );

    string strMailID         = MailID       ;

    string strCheck;
    strCheck = regex_replace( strMailID , regex( R"([0-9])" ) ,"" );
    if( strCheck != "" ){ return -1; }

    int mailStatus = 0;
    sqlite3_open16( dbPATH , &db );
    string sqlSelect = "select * from mailBox where GhostMenuName ='" + strGhostMenuName + "' and MailID = '" + strMailID + "'";
    int sqliteRes = sqlite3_exec( db , sqlSelect.c_str() , callbackStatusMail , (void*)&mailStatus , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif
    sqlite3_close( db );

    return mailStatus;
}


extern "C" __declspec(dllexport) HGLOBAL __cdecl request(HGLOBAL h, long *len){
    char req[*len+1];
    memset( req , '\0' , *len+1 );
    memcpy( req , (char*)h , *len );

//#ifdef Debug
//    printf( "%s----\n" , req );
//#endif

    GlobalFree( h );


    bool NOTIFY = false;
    resBuf = NULL;

    char* ID        = NULL;
    char* Sender    = NULL;


    char* Reference0 = NULL;
    char* Reference1 = NULL;
    char* Reference2 = NULL;
    char* Reference3 = NULL;
    char* Reference4 = NULL;
    char* Reference5 = NULL;
    char* Reference6 = NULL;
    char* Reference7 = NULL;

    char  sepLine[]    = "\r\n";
    char  sepLR[] = ": ";
    char* tp ;
    tp = strtok( req , sepLine );
    while( tp != NULL ){
        if ( tp == "GET PLUGIN/2.0" ){
            tp = strtok( NULL , sepLine );
            continue;
        } else if ( tp == "NOTIFY PLUGIN/2.0" ){
            NOTIFY = true;
            tp = strtok( NULL , sepLine );
            continue;
        }

        //左右分割を試みる。
        char* checkR = strstr( tp , sepLR );
        if ( checkR == NULL ){
            tp = strtok( NULL , sepLine );
            continue;
        }


        int Lsize = strlen( tp ) - strlen( checkR ) ;
        char L[ Lsize + 1 ] ;
        memset( L , '\0' , Lsize + 1 );
        memcpy( L , tp , Lsize);

        int Rsize = strlen( tp ) - ( Lsize + strlen( sepLR ) );
        //char R[ Rsize + 1 ] ;
        //memset( R , '\0' , Rsize + 1 );
        //memcpy( R , &tp[ Lsize + strlen( sepLR ) ] , Rsize);

        //printf( "%s\n" , L );
        //printf( "%s\n" , R );
        
        if ( strcmp( L , "ID" ) == 0 ) {
            ID = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( ID , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Sender" ) == 0 ) {
            Sender = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Sender , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference0" ) == 0 ) {
            Reference0 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference0 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference1" ) == 0 ) {
            Reference1 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference1 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference2" ) == 0 ) {
            Reference2 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference2 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference3" ) == 0 ) {
            Reference3 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference3 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference4" ) == 0 ) {
            Reference4 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference4 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference5" ) == 0 ) {
            Reference5 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference5 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference6" ) == 0 ) {
            Reference6 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference6 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        } else if ( strcmp( L , "Reference7" ) == 0 ) {
            Reference7 = (char*)calloc( Rsize + 1 , sizeof(char) );
            memcpy( Reference7 , &tp[ Lsize + strlen( sepLR ) ] , Rsize );

        //} else if ( strcomp( L , "" ) == 0 ) {
        }
        tp = strtok( NULL , sepLine );
    }



    if ( ID != NULL ) {

        if ( strcmp( ID , "version" ) == 0 ) {
            char res_buf[] = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nValue: 1.0.0\r\n\r\n";
            resBuf = res_buf;



        //起動時のメールチェック
        } else if ( strcmp( ID , "OnSecondChange" ) == 0 && bootOne == false ) {
            //処理が必要なのは、今の日付を取得してselect文を廻す必要があるが、
            bootOne = true;

            string strYMD;
            stringstream streamYMD;
            time_t t = time(nullptr);
            const tm* localTime = localtime(&t);
            streamYMD << localTime->tm_year + 1900;
            streamYMD << setw(2) << setfill('0') << localTime->tm_mon + 1;
            streamYMD << setw(2) << setfill('0') << localTime->tm_mday;
            streamYMD >> strYMD;

            char* err = NULL;
            sqlite3_open16( dbPATH , &db );
            newMailCount = 0;
            string newMailList = "select * from mailBox where YYYYmmdd <= " + strYMD + " and Checked = 0" ;
            stringstream streamMailCount ;

            int sqliteRes = sqlite3_exec( db , newMailList.c_str() , callbackNewMailCount , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif
            streamMailCount << newMailCount;
            //printf( "Select%s\n" , err );
            sqlite3_close( db );

            string total;
            if ( newMailCount > 0 ){
                string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: ";
                string strMailCount = "\\![set,trayballoon,--title=MailBox,--text=" + streamMailCount.str() + "件の未読メールがあります。]";
                string end          = "\r\nScriptOption: nobreak,notranslate\r\n\r\n";
                total               = start + strMailCount + end;
            } else {
                total = "PLUGIN/2.0 204 No Content\r\nCharset: UTF-8\r\n\r\n";
            }

            int i               = strlen( total.c_str() );

            char* res_buf;
            res_buf = (char*)calloc( i + 1 , sizeof(char) );
            memcpy( res_buf , total.c_str() , i );
            resBuf = res_buf;

            streamYMD.str("");
            streamYMD.clear( stringstream::goodbit );
            streamMailCount.str("");
            streamMailCount.clear( stringstream::goodbit );



        ////Ghost作者が使用する機能
        //メール送信機能
        } else if ( strcmp( ID , "OnSendMail" ) == 0 ) {
            if (! ( Sender == NULL || Reference0 == NULL || Reference1 == NULL || Reference2 == NULL || Reference3 == NULL || Reference4 == NULL || Reference5 == NULL || Reference6 == NULL )){
                SendMail( Sender , Reference0 ,  Reference1 , Reference2 , Reference3 , Reference4 , Reference5 , Reference6 );
            }


        //メール削除機能
        } else if ( strcmp( ID , "OnDeleteMail" ) == 0 ) {
            if ( Reference0 != NULL ){
                DeleteMail( Sender , Reference0 );
            }


        //メールの状態を確認する機能
        } else if ( strcmp( ID , "OnStatusMail" ) == 0 ) {
            if ( Reference0 != NULL ){
                int mailStatus = StatusMail( Sender , Reference0 );
                stringstream streamMailStatus; 
                streamMailStatus << mailStatus;
                string strMailStatus;
                string Rzero = Reference0;
                strMailStatus = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nEvent: OnMailStatus\r\nReference0: " + Rzero + "\r\nReference1: " + streamMailStatus.str() + "\r\n\r\n";
                streamMailStatus.str("");
                streamMailStatus.clear( stringstream::goodbit );


                int i = strlen( strMailStatus.c_str() );
                char* res_buf;
                res_buf = (char*)calloc( i + 1 , sizeof(char) );
                memcpy( res_buf , strMailStatus.c_str() , i );

                resBuf = res_buf;
            }

        ////Userが触る機能
        //メールボックス
        //┌ └ ┐ ┘ ├ ┤ ─ ┬ ┼ ┴
        //\\_a[OnCheckMail,0,0] ───未読メール─── \\_a
        //\\_a[OnCheckMail,1,0] ───既読メール─── \\_a┼
        } else if ( strcmp( ID , "OnMenuExec" ) == 0 ) {
            char res_buf[] = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\_q \\n┌┬────────────┬┐ \\n├┼────────────┼┤ \\n├┼\\_a[OnCheckMail,0,0] ───未読メール─── \\_a┼┤ \\n├┼────────────┼┤ \\n├┼────────────┼┤ \\n├┼\\_a[OnCheckMail,1,0] ───既読メール─── \\_a┼┤ \\n├┼────────────┼┤ \\n└┴────────────┴┘ \\_q \r\nScriptOption: nobreak,notranslate\r\n\r\n";
            //char res_buf[] = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\_q\\_a[OnCheckMail,0,0]未読メール\\_a\\n\\_a[OnCheckMail,1,0]既読メール\\_a\\_q\r\nScriptOption: nobreak,notranslate\r\n\r\n";
            resBuf = res_buf;


        //第1引数 未読 = 0 , 既読 = 1
        //第2引数 offset
        } else if ( strcmp( ID , "OnCheckMail" ) == 0 ) {
            if ( Reference0 != NULL && Reference1 != NULL ) {
                string strID        = ID;
                string strChecked   = Reference0 ;
                string strOffset    = Reference1 ;
                int offset          = atoi( Reference1 );

                string strYMD;
                stringstream streamYMD;
                time_t t = time(nullptr);
                const tm* localTime = localtime(&t);
                streamYMD << localTime->tm_year + 1900;
                streamYMD << setw(2) << setfill('0') << localTime->tm_mon + 1;
                streamYMD << setw(2) << setfill('0') << localTime->tm_mday;
                streamYMD >> strYMD;

                char* err = NULL;
                sqlite3_open16( dbPATH , &db );
                string mailList = "select * from mailBox where YYYYmmdd <= " + strYMD + " and Checked = " + strChecked + " limit 20 offset " + strOffset ;
                int sqliteRes = sqlite3_exec( db , mailList.c_str() , callbackMailList , (void*)&offset , &err );
#ifdef Debug
                if ( sqliteRes != 0 ){
                    printf( "Select%s\n" , err );
                }
#endif
                sqlite3_close( db );

                string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\0\\b[2]\\_q";
                string backSelect;
                if( offset < 20 ){
                    backSelect   = "┌┬──────最新──────\\n";
                } else {
                    backSelect   = "┌┬\\_a[OnCheckMail," + strChecked + "," + to_string( offset - 20 ) + "]─────前の20件─────\\_a\\n";
                }
                string nextSelect;
                nextSelect   = "└┴\\_a[OnCheckMail," + strChecked + "," + to_string( offset + 20 ) + "]─────次の20件─────\\_a\\n";

                string end          = "\\_q\r\nScriptOption: nobreak,notranslate\r\n\r\n";
                string selectRes    = s.str();
                string total        = start + backSelect + selectRes + nextSelect + end;
                int i               = strlen( total.c_str() );

                char* res_buf;
                res_buf = (char*)calloc( i + 1 , sizeof(char) );
                memcpy( res_buf , total.c_str() , i );
                resBuf = res_buf;


                streamYMD.str("");
                streamYMD.clear( stringstream::goodbit );
                s.str("");
                s.clear( stringstream::goodbit );
            }
        

        //|               |        |          |        |       |          |         |
        //| GhostMenuName | MailID | YYYYmmdd | Sender | Title | MailText | Checked |
        //|               |        |          |        |       |          |         |
        //第1引数 : ゴースト名
        //第2引数 : メールID
        //第3引数 : それらのオフセット
        //既読リストに限りオフセットが欲しい。
        } else if ( strcmp( ID , "OnOpenMail" ) == 0 ) {
            if ( Reference0 != NULL && Reference1 != NULL && Reference2 != NULL ){
                string strGhostMenuName = Reference0;
                string strMailID        = Reference1;
                string strOffSet        = Reference2;


                char* err = NULL;
                sqlite3_open16( dbPATH , &db );
                string moveMail = "update mailBox set Checked = 1 where GhostMenuName = '" + strGhostMenuName + "' and MailID = " + strMailID  ;
                int sqliteRes = sqlite3_exec( db , moveMail.c_str() , NULL , NULL , &err );
#ifdef Debug
                if ( sqliteRes != 0 ) {
                    printf( "%s\n" , err );
                }
#endif

                //printf( "Update%s\n" , err );
                string openMail = "select * from mailBox where GhostMenuName = '" + strGhostMenuName + "' and MailID = " + strMailID  ;
                sqliteRes = sqlite3_exec( db , openMail.c_str() , callbackOpenMail , NULL , &err );
#ifdef Debug
                if ( sqliteRes != 0 ) {
                    printf( "%s\n" , err );
                }
#endif
                //printf( "Select%s\n" , err );
                sqlite3_close( db );

                string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\0\\b[2]\\_q";
                string end          = "\\n\\_a[OnCheckMail,0,0]未読メール\\_a - \\_a[OnCheckMail,1," + strOffSet + "]既読メール\\_a\\_q \r\nScriptOption: nobreak,notranslate\r\n\r\n";
                string selectRes    = s.str();
                string total        = start + selectRes + end;
                int i               = strlen( total.c_str() );

                char* res_buf;
                res_buf = (char*)calloc( i + 1 , sizeof(char) );
                memcpy( res_buf , total.c_str() , i );
                resBuf = res_buf;

                s.str("");
                s.clear( stringstream::goodbit );
            }

        } else if ( strcmp( ID , "OnOtherGhostTalk" ) == 0 ) {
        //} else if ( strcmp( ID , "" ) == 0 ) {
        }
    }

    //返すものがなかった時
    if ( resBuf == NULL ){
        char res_buf[] = "PLUGIN/2.0 204 No Content\r\nCharset: UTF-8\r\n\r\n";
        resBuf = res_buf;
    }
    if ( ID != NULL ){ free( ID ); }
    if ( Sender != NULL ){ free( Sender ); }
    if ( Reference0 != NULL ){ free( Reference0 ); }
    if ( Reference1 != NULL ){ free( Reference1 ); }
    if ( Reference2 != NULL ){ free( Reference2 ); }
    if ( Reference3 != NULL ){ free( Reference3 ); }
    if ( Reference4 != NULL ){ free( Reference4 ); }
    if ( Reference5 != NULL ){ free( Reference5 ); }
    if ( Reference6 != NULL ){ free( Reference6 ); }
    if ( Reference7 != NULL ){ free( Reference7 ); }
    
    //pluginは2.0で返す。
    //char res_buf[] = "PLUGIN/2.0 204 No Content";
    *len = strlen(resBuf);
    HGLOBAL ret = GlobalAlloc(GPTR, (SIZE_T)(*len));
    memcpy(ret, (void*)(resBuf), *len);
    return ret;
}




