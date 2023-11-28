


#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <regex>

//sqlite
#include <sqlite3.h>

//stringstream
#include <iomanip>


//#define Debug





using namespace std;

FILE* ConsoleWindow;

typedef void* HGLOBAL;
char* resBuf;

char* pluginDirectory;
wchar_t* dbPATH;
sqlite3 *db = NULL;

static string strYMD;
static stringstream s;
static int SecChange = 60;


//{{{
int main(int argc, char* argv[]) {
    printf( "%s\n" , argv[0] );
    return 0;
}
//}}}
//{{{
extern "C" __declspec(dllexport) bool __cdecl load(HGLOBAL h, long len){
//hにはdllまでのLogFilePathが入っている。
//lenはアドレスの長さ。\0の分は入っていない。

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
    int res = sqlite3_open16( dbPATH , &db );
    int sqliteRes = sqlite3_exec( db , "create table mailBox2( GhostMenuName text , MailID int , yyyymmdd int , hour int , Sender text , Title text , MailText text , Checked int , Notified int );" , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif

    sqlite3_close( db );

    return true;
}
//}}}
//{{{
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
//}}}


/*{{{*/
// 共通関数
string Sanitize( string sanitize ){
    sanitize = regex_replace( sanitize , regex( "'" ) ,"" );
    sanitize = regex_replace( sanitize , regex( "vanishbymyself" ) ,"危険な文字" );
    return sanitize;
}
string ZenToHan( string str ){
    str = regex_replace( str , regex( "０" ) ,"0" );
    str = regex_replace( str , regex( "１" ) ,"1" );
    str = regex_replace( str , regex( "２" ) ,"2" );
    str = regex_replace( str , regex( "３" ) ,"3" );
    str = regex_replace( str , regex( "４" ) ,"4" );
    str = regex_replace( str , regex( "５" ) ,"5" );
    str = regex_replace( str , regex( "６" ) ,"6" );
    str = regex_replace( str , regex( "７" ) ,"7" );
    str = regex_replace( str , regex( "８" ) ,"8" );
    str = regex_replace( str , regex( "９" ) ,"9" );
    str = regex_replace( str , regex( "　" ) ,"" );
    str = regex_replace( str , regex( " " ) ,"" );
    return str;
}


/*}}}*/
// {{{
// sql Call Back 関数
//| 旧mailBox     |        |          |        |       |          |         | 
//| GhostMenuName | MailID | YYYYmmdd | Sender | Title | MailText | Checked |
//
//| mailBox2      |        |          |      |        |       |          |         |  
//| GhostMenuName | MailID | YYYYmmdd | hour | Sender | Title | MailText | Checked | Notified
//|               |        |          |      |        |       |          |         |
//anything = offset
int callbackMailList(void *anything, int keyCount, char **value, char **key){
    string Checked = value[7];
    //未読ならわかりやすく
    if ( Checked == "0" ){
        s << "├┼─\\q[" << value[4] << " : " << value[5] << ",OnOpenMail," << value[0] << "," << value[1] << "," << *(int*)anything << "]\\n";
    } else {
        s << "├┼\\q[" << value[4] << " : " << value[5] << ",OnOpenMail," << value[0] << "," << value[1] << "," << *(int*)anything << "]\\n";
    }
    return 0;
}
//anything = offset
int callbackDevMailList(void *anything, int keyCount, char **value, char **key){
    s << "├┼\\q[" << value[4] << " : " << value[2] << " : " << value[5] << ",OnDevMail," << value[0] << "," << value[1] << "," << *(int*)anything << "]\\n";
    return 0;
}

int callbackOpenMail(void *anything, int keyCount, char **value, char **key){
    s << "   【" << value[5] << "】" << "\\n\\n" << value[6] << "\\n\\n\\n   【" << value[0] << "】";
    return 0;
}
int callbackNewMailCount(void *anything, int keyCount, char **value, char **key){
    *(int*)anything = *(int*)anything + 1;
    return 0;
}
int callbackStatusMail(void *anything, int keyCount, char **value, char **key){
    string Checked      = value[7];
    string nowYYYYmmdd  = value[2];

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
    return 0;
}
int callbackAllMailID(void *anything, int keyCount, char **value, char **key){
    s << value[1] << ":";
    return 0;
}


/*}}}*/
/*{{{*/
// 基本関数
void SendMail( char* GhostMenuName , char* MailID , char* YYYY , char* MM , char* DD , char* Sender , char* Title , char* MailText ){
    //引数が正しくなくnullの可能性。足りない場合の処理
    char* err = NULL;

    string strGhostMenuName  = GhostMenuName;
    strGhostMenuName  = Sanitize( strGhostMenuName );

    string strMailID         = MailID       ;
    strMailID                = ZenToHan( strMailID );

    //二桁にすること。結合すること。
    string strYYYY           = YYYY         ;
    strYYYY                  = ZenToHan( strYYYY );
    string strMM             = MM           ;
    strMM                    = ZenToHan( strMM );
    string strDD             = DD           ;
    strDD                    = ZenToHan( strDD );

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
    strSender               = Sanitize( Sender );

    string strTitle         = Title        ;
    strTitle                = Sanitize( strTitle );

    string strMailText      = MailText     ;
    strMailText             = Sanitize( strMailText );



    sqlite3_open16( dbPATH , &db );
    int sqliteRes = sqlite3_exec( db , "create table mailBox2( GhostMenuName text , MailID int , yyyymmdd int , hour int , Sender text , Title text , MailText text , Checked int , Notified int );" , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif

    string sqlDelete = "delete from mailBox2 where GhostMenuName ='" + strGhostMenuName + "' and MailID = '" + strMailID + "'";
    sqliteRes = sqlite3_exec( db , sqlDelete.c_str() , NULL , NULL , &err );
    //sqliteRes = sqlite3_exec( db , sqlDelete.c_str() , NULL , NULL , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif

    string sqlInsert = "insert into mailBox2 values( '" + strGhostMenuName + "' , " + strMailID + " , " + strYYYYmmdd + " , 0 , '" + strSender + "' , '" + strTitle + "' , '" + strMailText + "' , 0 , 0 )"; 
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
    strGhostMenuName         = Sanitize( strGhostMenuName );

    string strMailID         = MailID       ;
    strMailID                = ZenToHan( strMailID );

    string strCheck;
    strCheck = regex_replace( strMailID , regex( R"([0-9])" ) ,"" );
    if( strCheck != "" ){ return; }

    sqlite3_open16( dbPATH , &db );
    string sqlDelete = "delete from mailBox2 where GhostMenuName ='" + strGhostMenuName + "' and MailID = '" + strMailID + "'";
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
    strGhostMenuName         = Sanitize( strGhostMenuName );

    string strMailID         = MailID       ;
    strMailID                = ZenToHan( strMailID );

    string strCheck;
    strCheck = regex_replace( strMailID , regex( R"([0-9])" ) ,"" );
    if( strCheck != "" ){ return -1; }

    strYMD = "";
    stringstream streamYMD;
    time_t t = time(nullptr);
    const tm* localTime = localtime(&t);
    streamYMD << localTime->tm_year + 1900;
    streamYMD << setw(2) << setfill('0') << localTime->tm_mon + 1;
    streamYMD << setw(2) << setfill('0') << localTime->tm_mday;
    streamYMD >> strYMD;
    streamYMD.str("");
    streamYMD.clear( stringstream::goodbit );


    int mailStatus = 0;
    sqlite3_open16( dbPATH , &db );
    string sqlSelect = "select * from mailBox2 where GhostMenuName ='" + strGhostMenuName + "' and MailID = '" + strMailID + "'";
    int sqliteRes = sqlite3_exec( db , sqlSelect.c_str() , callbackStatusMail , (void*)&mailStatus , &err );
#ifdef Debug
    if ( sqliteRes != 0 ) {
        printf( "%s\n" , err );
    }
#endif
    sqlite3_close( db );

    return mailStatus;
}
/*}}}*/


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

        //{{{
        ////新着メール通知機能
        //1秒ないし、短期間でループさせる。
        //回転数はこっちで制限を用意して決める。
        //一時間に一回程度で大丈夫だけど、
        //起動時にほしい。
        } else if ( strcmp( ID , "OnSecondChange" ) == 0 ) {
            SecChange++;
            //一分毎に実行
            //一時間ごとにチェック。初回は起動時。
            if ( SecChange > 60){
                SecChange = 0;

                string strYMD;
                stringstream streamYMD;
                time_t t = time(nullptr);
                const tm* localTime = localtime(&t);
                streamYMD << localTime->tm_year + 1900;
                streamYMD << setw(2) << setfill('0') << localTime->tm_mon + 1;
                streamYMD << setw(2) << setfill('0') << localTime->tm_mday;
                streamYMD >> strYMD;
                streamYMD.str("");
                streamYMD.clear( stringstream::goodbit );

                char* err = NULL;
                sqlite3_open16( dbPATH , &db );
                int newMailCount = 0;
                string newMailList = "select * from mailBox2 where YYYYmmdd <= " + strYMD + " and Notified = 0" ;
                int sqliteRes = sqlite3_exec( db , newMailList.c_str() , callbackNewMailCount , (void*)&newMailCount , &err );
                string moveMail = "update mailBox2 set Notified = 1 where YYYYmmdd <= " + strYMD + " and Notified = 0" ;
                sqliteRes = sqlite3_exec( db , moveMail.c_str() , NULL , NULL , &err );

                sqlite3_close( db );

                if ( newMailCount > 0 ){
                    stringstream x;
                    x << newMailCount ;

                    string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: ";
                    string strMailCount = "\\C\\![set,trayballoon,--title=MailBox,--text=" + x.str() + "件の新着メールがあります。,--icon=none]";
                    string end          = "\r\nScriptOption: nobreak,notranslate\r\n\r\n";
                    string total        = start + strMailCount + end;
                    x.str("");
                    x.clear( stringstream::goodbit );


                    int i = strlen( total.c_str() );
                    char* res_buf;
                    res_buf = (char*)calloc( i + 1 , sizeof(char) );
                    memcpy( res_buf , total.c_str() , i );
                    resBuf = res_buf;



                }
            }





        //}}}
        //{{{
        ////プラグインの存在を通知する機能
        } else if ( strcmp( ID , "OnGhostBoot" ) == 0 ) {
            string OnExistPluginMailBox = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nEvent: OnExistPluginMailBox\r\n\r\n";
            int i = strlen( OnExistPluginMailBox.c_str() );
            char* res_buf;
            res_buf = (char*)calloc( i + 1 , sizeof(char) );
            memcpy( res_buf , OnExistPluginMailBox.c_str() , i );
            resBuf = res_buf;




        //}}}
        //{{{
        ////Ghost作者が使用する機能
        //メール送信機能
        } else if ( strcmp( ID , "OnSendMail" ) == 0 ) {
            if (! ( Sender == NULL || Reference0 == NULL || Reference1 == NULL || Reference2 == NULL || Reference3 == NULL || Reference4 == NULL || Reference5 == NULL || Reference6 == NULL )){
                SendMail( Sender , Reference0 ,  Reference1 , Reference2 , Reference3 , Reference4 , Reference5 , Reference6 );
            }


        //メールの送信をステータスが0 の時に行う。
        } else if ( strcmp( ID , "OnSendMailNotUpdate" ) == 0 ) {
            int mailStatus = StatusMail( Sender , Reference0 );
            if( mailStatus == 0 ){
                if (! ( Sender == NULL || Reference0 == NULL || Reference1 == NULL || Reference2 == NULL || Reference3 == NULL || Reference4 == NULL || Reference5 == NULL || Reference6 == NULL )){
                    SendMail( Sender , Reference0 ,  Reference1 , Reference2 , Reference3 , Reference4 , Reference5 , Reference6 );
                }
            }

        //メールの送信をステータスが0 or 1 の時に行う。
        } else if ( strcmp( ID , "OnSendMailNotArrive" ) == 0 ) {
            int mailStatus = StatusMail( Sender , Reference0 );
            if( mailStatus == 0 || mailStatus == 1 ){
                if (! ( Sender == NULL || Reference0 == NULL || Reference1 == NULL || Reference2 == NULL || Reference3 == NULL || Reference4 == NULL || Reference5 == NULL || Reference6 == NULL )){
                    SendMail( Sender , Reference0 ,  Reference1 , Reference2 , Reference3 , Reference4 , Reference5 , Reference6 );
                }
            }


        //メール削除機能
        } else if ( strcmp( ID , "OnDeleteMail" ) == 0 ) {
            if ( Reference0 != NULL ){
                DeleteMail( Sender , Reference0 );
            }


        //未達ならメールを削除する。
        //引数0 : メールID
        } else if ( strcmp( ID , "OnDeleteMailNotArrive" ) == 0 ) {
            if ( Reference0 != NULL ){
                int mailStatus = StatusMail( Sender , Reference0 );
                if( mailStatus == 1 ){
                    DeleteMail( Sender , Reference0 );
                }
            }

        //メールの状態を確認する機能
        //第0引数 メールID
        } else if ( strcmp( ID , "OnStatusMail" ) == 0 ) {
            if ( Reference0 != NULL ){
                int mailStatus = StatusMail( Sender , Reference0 );
                stringstream streamMailStatus; 
                streamMailStatus << mailStatus;
                string strMailStatus;

                string R0 = Reference0;
                R0        = ZenToHan( R0 );

                strMailStatus = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nEvent: OnMailStatus\r\nReference0: " + R0 + "\r\nReference1: " + streamMailStatus.str() + "\r\n\r\n";
                streamMailStatus.str("");
                streamMailStatus.clear( stringstream::goodbit );


                int i = strlen( strMailStatus.c_str() );
                char* res_buf;
                res_buf = (char*)calloc( i + 1 , sizeof(char) );
                memcpy( res_buf , strMailStatus.c_str() , i );

                resBuf = res_buf;
            }

        //メールの状態を確認する機能
        //引数1-4  横流し
        //引数  5  メールID
        //引数  6  横流し
        //
        //返り値0-4:横流し
        //返り値5  :メールID
        //返り値6  :その結果
        //返り値7  :横流し
        } else if ( strcmp( ID , "OnStatusMailEX" ) == 0 ) {
            if ( Reference5 != NULL ){

                string strR0 = ""; 
                string strR1 = ""; 
                string strR2 = ""; 
                string strR3 = ""; 
                string strR4 = ""; 
                string strR7 = ""; 

                if ( Reference0 != NULL ){ strR0 = Reference0; } 
                if ( Reference1 != NULL ){ strR1 = Reference1; } 
                if ( Reference2 != NULL ){ strR2 = Reference2; } 
                if ( Reference3 != NULL ){ strR3 = Reference3; } 
                if ( Reference4 != NULL ){ strR4 = Reference4; } 
                if ( Reference6 != NULL ){ strR7 = Reference6; } 

                //mailID
                string strR5 = Reference5;
                strR5        = ZenToHan( strR5 );

                strR0 = "Reference0: " + strR0 + "\r\n"; 
                strR1 = "Reference1: " + strR1 + "\r\n";
                strR2 = "Reference2: " + strR2 + "\r\n";
                strR3 = "Reference3: " + strR3 + "\r\n";
                strR4 = "Reference4: " + strR4 + "\r\n";
                strR5 = "Reference5: " + strR5 + "\r\n";

                strR7 = "Reference7: " + strR7 + "\r\n";

                int mailStatus = StatusMail( Sender , Reference5 );
                stringstream streamMailStatus; 
                streamMailStatus << mailStatus;
                string strMailStatus;
                strMailStatus = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nEvent: OnMailStatusEX\r\n" + strR0 + strR1 + strR2 + strR3 + strR4 + strR5 + "Reference6: " + streamMailStatus.str() + "\r\n" + strR7 + "\r\n";
                streamMailStatus.str("");
                streamMailStatus.clear( stringstream::goodbit );


                int i = strlen( strMailStatus.c_str() );
                char* res_buf;
                res_buf = (char*)calloc( i + 1 , sizeof(char) );
                memcpy( res_buf , strMailStatus.c_str() , i );

                resBuf = res_buf;
            }


        //引数-区切りで
        //第0引数 : メールID-メールID-メールID-...
        //返却先  : OnMailsStatus
        //返り値0 : 受けた引数0
        //返り値1 : ステータス-ステータス-ステータス---...
        } else if ( strcmp( ID , "OnStatusMails" ) == 0 ) {
            if ( Reference0 != NULL ){

                string R0 = Reference0;
                R0        = ZenToHan( R0 );

                char* MailID ;
                char  statusMailsSep[]    = ":";
                MailID = strtok( Reference0 , statusMailsSep );
                stringstream a;
                while( MailID != NULL ){
                    int mailStatus = StatusMail( Sender , MailID );
                    a << mailStatus << ":";
                    MailID = strtok( NULL , statusMailsSep );
                }
                string mailsStatus ; 
                mailsStatus = regex_replace( a.str().c_str() , regex( ":$" ) ,"" );
                a.str("");
                a.clear( stringstream::goodbit );
                
                if ( mailsStatus != "" ){
                    string strMailsStatus;
                    strMailsStatus = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nEvent: OnMailsStatus\r\nReference0: " + R0 + "\r\nReference1: " + mailsStatus + "\r\n\r\n";
                    int i = strlen( strMailsStatus.c_str() );
                    char* res_buf;
                    res_buf = (char*)calloc( i + 1 , sizeof(char) );
                    memcpy( res_buf , strMailsStatus.c_str() , i );

                    resBuf = res_buf;
                }
            }


        //要求したゴーストのが今まで送信したすべてのIDを取得する。
        } else if ( strcmp( ID , "OnGetAllMailID" ) == 0 ) {
            char* err = NULL;
            string strGhostMenuName  = Sender;
            strGhostMenuName         = Sanitize( strGhostMenuName );

            sqlite3_open16( dbPATH , &db );
            string sqlSelect = "select * from mailBox2 where GhostMenuName ='" + strGhostMenuName + "' order by MailID asc";
            int sqliteRes = sqlite3_exec( db , sqlSelect.c_str() , callbackAllMailID, NULL , &err );

#ifdef Debug
            if ( sqliteRes != 0 ) {
                printf( "%s\n" , err );
            }
#endif
            sqlite3_close( db );

            string MailIDs = regex_replace( s.str() , regex( ":$" ) ,"" );
            s.str("");
            s.clear( stringstream::goodbit );

            if ( MailIDs != "" ){
                string strGetAllMailID;
                strGetAllMailID = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nEvent: OnAllMailID\r\nReference0: " + MailIDs + "\r\n\r\n";
                int i = strlen( strGetAllMailID.c_str() );
                char* res_buf;
                res_buf = (char*)calloc( i + 1 , sizeof(char) );
                memcpy( res_buf , strGetAllMailID.c_str() , i );

                resBuf = res_buf;
            }





        //}}}
        //{{{
        ////Userが触る機能
        //メールボックス
        //┌ └ ┐ ┘ ├ ┤ ─ ┬ ┼ ┴
        //\\_a[OnCheckMail,0,0] ───未読メール─── \\_a
        //\\_a[OnCheckMail,1,0] ───既読メール─── \\_a┼
        //\\_a[OnCheckMail,2,0] ───選択メール─── \\_a┼
        } else if ( strcmp( ID , "OnMenuExec" ) == 0 ) {
            char res_buf[] = 
                "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\_q┌┬────────────┬┐ \\n├┼────────────┼┤ \\n├┼ \\q[───未読メール───,OnCheckMail,0,0] ┼┤ \\n├┼────────────┼┤ \\n├┼ \\q[───既読メール───,OnCheckMail,1,0] ┼┤ \\n├┼────────────┼┤ \\n├┼ \\q[───個別メール───,OnCheckMail,2,0] ┼┤ \\n├┼────────────┼┤ \\n├┼────────────┼┤ \\n├┼ \\q[────閉じる────,] ┼┤ \\n└┴────────────┴┘ \\_q \r\nScriptOption: nobreak,notranslate\r\n\r\n";
            //char res_buf[] = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\_q\\_a[OnCheckMail,0,0]未読メール\\_a\\n\\_a[OnCheckMail,1,0]既読メール\\_a\\_q\r\nScriptOption: nobreak,notranslate\r\n\r\n";
            resBuf = res_buf;



        //第0引数 未読 = 0 , 既読 = 1 , そのゴーストのメール = 2
        //第1引数 offset
        } else if ( strcmp( ID , "OnCheckMail" ) == 0 ) {
            if ( Reference0 != NULL && Reference1 != NULL ) {

                string GhostMenuName = Sender;
                GhostMenuName = Sanitize( GhostMenuName );

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

                //ghost個別確認
                string mailList ;
                if ( strChecked == "2" ){
                    mailList = "select * from mailBox2 where GhostMenuName =='" + GhostMenuName + "' and YYYYmmdd <= " + strYMD + " order by YYYYmmdd desc,MailID desc limit 20 offset " + strOffset ;

                //未読 or 既読
                } else {
                    mailList = "select * from mailBox2 where YYYYmmdd <= " + strYMD + " and Checked = " + strChecked + " order by YYYYmmdd desc,MailID desc limit 20 offset " + strOffset ;
                }
                //string mailList = "select * from mailBox2 where YYYYmmdd <= " + strYMD + " and Checked = " + strChecked + " order by YYYYmmdd desc,MailID desc limit 20 offset " + strOffset ;
                int sqliteRes = sqlite3_exec( db , mailList.c_str() , callbackMailList , (void*)&offset , &err );
#ifdef Debug
                if ( sqliteRes != 0 ){
                    printf( "Select%s\n" , err );
                }
#endif
                //未読ディレクトリなら通知済みに変更
                if ( strChecked == "0" ){
                    string moveMail = "update mailBox2 set Notified = 1 where YYYYmmdd <= " + strYMD + " and Notified = 0" ;
                    sqliteRes = sqlite3_exec( db , moveMail.c_str() , NULL , NULL , &err );

                //個別ディレクトリならそのゴーストの未通知を通知に。
                } else if ( strChecked == "2" ){
                    string moveMail = "update mailBox2 set Notified = 1 where GhostMenuName =='" + GhostMenuName + "' and YYYYmmdd <= " + strYMD + " and Notified = 0" ;
                    sqliteRes = sqlite3_exec( db , moveMail.c_str() , NULL , NULL , &err );
                }

                sqlite3_close( db );

                string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\0\\b[2]\\_q";
                string backSelect;
                if( offset < 20 ){
                    backSelect   = "┌┬──────最新──────┬┐\\n";
                } else {
                    backSelect   = "┌┬\\_a[OnCheckMail," + strChecked + "," + to_string( offset - 20 ) + "]─────前の20件─────\\_a┬┐\\n";
                }
                string exitSelect;
                if( strChecked == "0" ){
                    exitSelect = "┌┬\\q[─────閉じる──────,]┬┐\\n├┼\\q[────既読メール─────,OnCheckMail,1,0]┼┤\\n└┴\\q[────個別メール─────,OnCheckMail,2,0]┴┘\\n\\n";
                } else {
                    exitSelect = "┌┬\\q[─────閉じる──────,]┬┐\\n├┼\\q[────未読メール─────,OnCheckMail,0,0]┼┤\\n└┴\\q[────個別メール─────,OnCheckMail,2,0]┴┘\\n\\n";
                }
                string nextSelect;
                nextSelect   = "└┴\\_a[OnCheckMail," + strChecked + "," + to_string( offset + 20 ) + "]─────次の20件─────\\_a┴┘\\n";

                string end          = "\\_q\r\nScriptOption: nobreak,notranslate\r\n\r\n";
                string selectRes    = s.str();
                string total        = start + exitSelect + backSelect + selectRes + nextSelect + end;
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
        //第0引数 : ゴースト名
        //第1引数 : メールID
        //第2引数 : それらのオフセット
        //既読リストに限りオフセットが欲しい。
        } else if ( strcmp( ID , "OnOpenMail" ) == 0 ) {
            if ( Reference0 != NULL && Reference1 != NULL && Reference2 != NULL ){
                string strGhostMenuName = Reference0;
                string strMailID        = Reference1;
                string strOffSet        = Reference2;


                char* err = NULL;
                sqlite3_open16( dbPATH , &db );
                string moveMail = "update mailBox2 set Checked = 1 where GhostMenuName = '" + strGhostMenuName + "' and MailID = " + strMailID  ;
                int sqliteRes = sqlite3_exec( db , moveMail.c_str() , NULL , NULL , &err );
#ifdef Debug
                if ( sqliteRes != 0 ) {
                    printf( "%s\n" , err );
                }
#endif

                //printf( "Update%s\n" , err );
                string openMail = "select * from mailBox2 where GhostMenuName = '" + strGhostMenuName + "' and MailID = " + strMailID  ;
                sqliteRes = sqlite3_exec( db , openMail.c_str() , callbackOpenMail , NULL , &err );
#ifdef Debug
                if ( sqliteRes != 0 ) {
                    printf( "%s\n" , err );
                }
#endif
                //printf( "Select%s\n" , err );
                sqlite3_close( db );

                string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\0\\b[2]\\_q";
                string end          = "\\n\\_a[OnCheckMail,0,0]未読メール\\_a - \\_a[OnCheckMail,1," + strOffSet + "]既読メール\\_a - \\_a[OnCheckMail,2," + strOffSet + "]個別メール\\_a \r\nScriptOption: nobreak,notranslate\r\n\r\n";
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







        //}}}
        //{{{
        ////開発者向けメニューを作成する。
        //引数 0 : オフセット
        //見れるメールはすべてのゴーストのメール
        //今日より先のメール
        } else if ( strcmp( ID , "OnDevList" ) == 0 ) {

            if ( Reference0 != NULL ) {
                string strOffset    = Reference0 ;
                int offset          = atoi( Reference0 );

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

                //ghost個別確認
                string mailList ;
                mailList = "select * from mailBox2 where  YYYYmmdd > " + strYMD + " order by YYYYmmdd desc,MailID desc limit 20 offset " + strOffset ;

                int sqliteRes = sqlite3_exec( db , mailList.c_str() , callbackDevMailList , (void*)&offset , &err );
#ifdef Debug
                if ( sqliteRes != 0 ){
                    printf( "Select%s\n" , err );
                }
#endif
                sqlite3_close( db );

                string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\0\\b[2]\\_q";
                string backSelect;
                if( offset < 20 ){
                    backSelect   = "┌┬──────最新──────┬┐\\n";
                } else {
                    backSelect   = "┌┬\\_a[OnDevList," + to_string( offset - 20 ) + "]─────前の20件─────\\_a┬┐\\n";
                }
                string exitSelect;
                exitSelect = "┌┬\\q[─────閉じる──────,]┬┐\\n└┴\\──────────────┴┘\\n\\n";

                string nextSelect;
                nextSelect   = "└┴\\_a[OnDevList," + to_string( offset + 20 ) + "]─────次の20件─────\\_a┴┘\\n";

                string end          = "\\_q\r\nScriptOption: nobreak,notranslate\r\n\r\n";
                string selectRes    = s.str();
                string total        = start + exitSelect + backSelect + selectRes + nextSelect + end;
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
        //第0引数 : ゴースト名
        //第1引数 : メールID
        //第2引数 : それらのオフセット
        } else if ( strcmp( ID , "OnDevMail" ) == 0 ) {
            if ( Reference0 != NULL && Reference1 != NULL && Reference2 != NULL ){
                string strGhostMenuName = Reference0;
                string strMailID        = Reference1;
                string strOffSet        = Reference2;


                char* err = NULL;
                sqlite3_open16( dbPATH , &db );

                //printf( "Update%s\n" , err );
                string openMail = "select * from mailBox2 where GhostMenuName = '" + strGhostMenuName + "' and MailID = " + strMailID  ;
                int sqliteRes = sqlite3_exec( db , openMail.c_str() , callbackOpenMail , NULL , &err );
#ifdef Debug
                if ( sqliteRes != 0 ) {
                    printf( "%s\n" , err );
                }
#endif
                //printf( "Select%s\n" , err );
                sqlite3_close( db );

                string start        = "PLUGIN/2.0 200 OK\r\nCharset: UTF-8\r\nScript: \\0\\b[2]\\_q";
                string end          = "\\n\\_a[OnDevList," + strOffSet + "]開発メール\\_a\r\nScriptOption: nobreak,notranslate\r\n\r\n";
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




        //}}}

        //} else if ( strcmp( ID , "OnOtherGhostTalk" ) == 0 && NewMail != 0 ) {



        } else if ( strcmp( ID , "OnGhostExit" ) == 0 ) {
#ifdef Debug
            printf( "%s\n" , Reference0 );
            printf( "%s\n" , Reference1 );
            printf( "%s\n" , Reference2 );
            printf( "%s\n" , Reference3 );
            printf( "%s\n" , Reference4 );
#endif

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




