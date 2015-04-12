#include "spreadmanager.h"
#include <QtConcurrent/QtConcurrent>
#include <string>
#include <QDebug>
#include "notificationmanager.h"
#include "singleton.h"
#include "qmessagebox.h"

SpreadManager::SpreadManager(QObject *parent) : QObject(parent)
{

}

SpreadManager::~SpreadManager()
{

}

void SpreadManager::initConnection(QString _port, QString _name, QString _group){

    this->name = _name;
    char *group = toChar(_group);
    char *port = toChar(_port);
    char *name = toChar(_name);

    group_name = group;
    int	ret;
    sp_time test_timeout;

    test_timeout.sec = 5;
    test_timeout.usec = 0;

    ret = SP_connect_timeout( port, name, 0, 1, &Mbox, Private_group, test_timeout );

    if( ret != ACCEPT_SESSION )
    {
        SP_error( ret );
        QString errMess = decryptErrorMessage(ret);

        NotificationManager *notm = &Singleton<NotificationManager>::Instance();
        std::function<void (int)> fp = [](int a) { Q_UNUSED(a); };


//        std::function<void (QMessageBox)> butFunc = [](QMessageBox mbox)->void { mbox.setStandardButtons(QMessageBox::Ok);};
        notm->showAlert(errMess,fp);
//        NotificationManager::showAlert(errMess,fp);
        return;
    }

    ret = SP_join(Mbox,group_name);
    if( ret != 0 )
    {
        SP_error( ret );
        Bye();

    }
    connected = true;

    emit didConnect();

    QtConcurrent::run(this,&SpreadManager::Read_thread_routine);

    qDebug("User: connected to %s with private group %s\n", port, Private_group);
}


void SpreadManager::closeConnection(){

    int t = SP_disconnect( Mbox );
    qDebug("disconected:%d",t);
    connected = false;

    emit didDisconnect();
}

char* SpreadManager::toChar(QString str){

    char* cstr;
    std::string fname = str.toStdString();
    cstr = new char [fname.size()+1];
    strcpy( cstr, fname.c_str() );
    return cstr;
}

void SpreadManager::Read_message(int fd, int code, void *data)
{

    Q_UNUSED(fd);
    Q_UNUSED(code);
    Q_UNUSED(data);

    static	char		 mess[MAX_MESSLEN];
    char		 sender[MAX_GROUP_NAME];    
    char		 target_groups[MAX_MEMBERS][MAX_GROUP_NAME];
    membership_info  memb_info;
    vs_set_info      vssets[MAX_VSSETS];
    unsigned int     my_vsset_index;
    int              num_vs_sets;
    char             members[MAX_MEMBERS][MAX_GROUP_NAME];
    int		 num_groups;
    int		 service_type;
    int16		 mess_type;
    int		 endian_mismatch;
    int		 i,j;
    int		 ret;

    service_type = 0;

    ret = SP_receive( Mbox, &service_type, sender, 100, &num_groups, target_groups,
        &mess_type, &endian_mismatch, sizeof(mess), mess );
    printf("\n============================\n");

    QString qs = QString(sender);
    QStringList strList = qs.split('#');

    QString qsender;
    if(strList.count()>1){
        qsender = strList[1];

    }


    if( ret < 0 )
    {
                if ( (ret == GROUPS_TOO_SHORT) || (ret == BUFFER_TOO_SHORT) ) {
                        service_type = DROP_RECV;
                        printf("\n========Buffers or Groups too Short=======\n");
                        ret = SP_receive( Mbox, &service_type, sender, MAX_MEMBERS, &num_groups, target_groups,
                                          &mess_type, &endian_mismatch, sizeof(mess), mess );
                }
        }
        if (ret < 0 )
        {
        if( ! To_exit )
        {
            SP_error( ret );
            printf("\n============================\n");
            printf("\nBye.\n");
        }
        exit( 0 );
    }
    if( Is_regular_mess( service_type ) )
    {
        mess[ret] = 0;
        if     ( Is_unreliable_mess( service_type ) ) printf("received UNRELIABLE ");
        else if( Is_reliable_mess(   service_type ) ) printf("received RELIABLE ");
        else if( Is_fifo_mess(       service_type ) ) printf("received FIFO ");
        else if( Is_causal_mess(     service_type ) ) printf("received CAUSAL ");
        else if( Is_agreed_mess(     service_type ) ) printf("received AGREED ");
        else if( Is_safe_mess(       service_type ) ) printf("received SAFE ");
        printf("message from %s, of type %d, (endian %d) to %d groups \n(%d bytes): %s\n",
            sender, mess_type, endian_mismatch, num_groups, ret, mess );


        if(qsender != name){
            emit messReceived(mess);
        }

//        SpreadManager->messReceived(mess_type, mess);
    }else if( Is_membership_mess( service_type ) )
        {
                ret = SP_get_memb_info( mess, service_type, &memb_info );
                if (ret < 0) {
                        printf("BUG: membership message does not have valid body\n");
                        SP_error( ret );
                        exit( 1 );
                }
        if     ( Is_reg_memb_mess( service_type ) )
        {
            printf("Received REGULAR membership for group %s with %d members, where I am member %d:\n",
                sender, num_groups, mess_type );
            for( i=0; i < num_groups; i++ )
                printf("\t%s\n", &target_groups[i][0] );
            printf("grp id is %d %d %d\n",memb_info.gid.id[0], memb_info.gid.id[1], memb_info.gid.id[2] );

            if( Is_caused_join_mess( service_type ) )
            {
                printf("Due to the JOIN of %s\n", memb_info.changed_member );
            }else if( Is_caused_leave_mess( service_type ) ){
                printf("Due to the LEAVE of %s\n", memb_info.changed_member );
            }else if( Is_caused_disconnect_mess( service_type ) ){
                printf("Due to the DISCONNECT of %s\n", memb_info.changed_member );
            }else if( Is_caused_network_mess( service_type ) ){
                printf("Due to NETWORK change with %u VS sets\n", memb_info.num_vs_sets);
                                num_vs_sets = SP_get_vs_sets_info( mess, &vssets[0], MAX_VSSETS, &my_vsset_index );
                                if (num_vs_sets < 0) {
                                        printf("BUG: membership message has more then %d vs sets. Recompile with larger MAX_VSSETS\n", MAX_VSSETS);
                                        SP_error( num_vs_sets );
                                        exit( 1 );
                                }
                                for( i = 0; i < num_vs_sets; i++ )
                                {
                                        printf("%s VS set %d has %u members:\n",
                                               (i  == my_vsset_index) ?
                                               ("LOCAL") : ("OTHER"), i, vssets[i].num_members );
                                        ret = SP_get_vs_set_members(mess, &vssets[i], members, MAX_MEMBERS);
                                        if (ret < 0) {
                                                printf("VS Set has more then %d members. Recompile with larger MAX_MEMBERS\n", MAX_MEMBERS);
                                                SP_error( ret );
                                                exit( 1 );
                                        }
                                        for( j = 0; j < (int) vssets[i].num_members; j++ )
                                                printf("\t%s\n", members[j] );
                                }
            }
        }else if( Is_transition_mess(   service_type ) ) {
            printf("received TRANSITIONAL membership for group %s\n", sender );
        }else if( Is_caused_leave_mess( service_type ) ){
            printf("received membership message that left group %s\n", sender );
        }else printf("received incorrecty membership message of type 0x%x\n", service_type );
        } else if ( Is_reject_mess( service_type ) )
        {
        printf("REJECTED message from %s, of servicetype 0x%x messtype %d, (endian %d) to %d groups \n(%d bytes): %s\n",
            sender, service_type, mess_type, endian_mismatch, num_groups, ret, mess );
    }else printf("received message of unknown message type 0x%x with ret %d\n", service_type, ret);


    printf("\n");
    fflush(stdout);
}

void SpreadManager:: Read_thread_routine()
{

    while(connected)
//    forever
    {
//        if(!connected){
//            qDebug("break");
//            return;
//        }
        Read_message(0,0,NULL);
    }
    qDebug("break\n\n\n\n\\n\n\n\n");
    return ;
}


void SpreadManager::sendMes(QString m){

    const char *mess = m.toStdString().c_str();
    int m_len = strlen(mess);
    int ret = SP_multicast(Mbox, SAFE_MESS, group_name, 1, m_len, mess);
    if( ret < 0 ) {
        SP_error( CONNECTION_CLOSED );
        Bye();
    }
}

void SpreadManager::Bye()
{
    To_exit = 1;
    printf("\nBye.\n");
    closeConnection();
//    pthread_join( Read_pthread, NULL );
    exit( 0 );
}


// drawing

void SpreadManager::startDrawing(QPoint p){
    QString mess = QString("com0 x=%1 y=%2").arg(QString::number(p.x()),QString::number(p.y()));
    sendMes(mess);
}


void SpreadManager::continueDrawing(QPoint p){
    QString mess = QString("com1 x=%1 y=%2").arg(QString::number(p.x()),QString::number(p.y()));
    sendMes(mess);
}


void SpreadManager::stopDrawing(QPoint p){
    QString mess = QString("com2 x=%1 y=%2").arg(QString::number(p.x()),QString::number(p.y()));
    sendMes(mess);
}


QString SpreadManager::decryptErrorMessage(int errNum){
    QString mess;
    switch (errNum) {
    case ILLEGAL_SPREAD:
        mess = "Illegal Spread.";
        break;
    case COULD_NOT_CONNECT:
        mess = "Could not connect.";
        break;
    case REJECT_NO_NAME:
        mess = "Name not specified.";
        break;
    case REJECT_NOT_UNIQUE:
        mess = "Name not unique.";
        break;
    case REJECT_ILLEGAL_NAME:
        mess = "Illegal name.";
        break;
    case CONNECTION_CLOSED:
        mess = "Connection closed.";
        break;
    case REJECT_AUTH:
        mess = "Reject Auth data.";
        break;
    case ILLEGAL_GROUP:
        mess = "Illegal group.";
        break;
    case ILLEGAL_MESSAGE:
        mess = "Illegal message.";
        break;
    case ILLEGAL_SERVICE:
        mess = "Illegal port.";
        break;
    case ILLEGAL_SESSION:
        mess = "Illegal session.";
        break;
    case NET_ERROR_ON_SESSION:
        mess = "Net error on session =(";
        break;
    case MESSAGE_TOO_LONG:
        mess = "Message too long.";
        break;
    default:
        mess = QString("Unknown error: %d").arg(errNum);
        break;
    }
    return mess;
}

