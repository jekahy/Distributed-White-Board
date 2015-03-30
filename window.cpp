#include "window.h"
#include "ui_window.h"
#include "sp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    numOfLines = 0;

    this->setup();
}

Window::~Window()
{
    delete ui;
}


void Window::paintEvent(QPaintEvent *e){

    Q_UNUSED(e)
    setAttribute(Qt::WA_OpaquePaintEvent);

    QPainter painter(this);
    QPen pointPen(Qt::blue);
    pointPen.setWidth(3);

    pointPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pointPen);

    if (!points.isEmpty()){

        if (points.count() > 1){
            QLine line(points[points.count()-2], points[points.count()-1]);
            painter.drawLine(line);

        }else{
            painter.drawPoint(points.last());
        }
    }
}


void Window::mousePressEvent(QMouseEvent *e){

    mousePressed = true;
    points.append(e->pos());
    update();
}

void Window::mouseReleaseEvent(QMouseEvent *e){
    Q_UNUSED(e)
    mousePressed = false;
    p_arr.append(points);
    points.clear();
}

void Window::mouseMoveEvent(QMouseEvent *e){

    if (mousePressed){
        points.append(e->pos());
        update();
    }
}

#define MAX_MESSLEN     102400
#define MAX_VSSETS      10
#define MAX_MEMBERS     100


static  pthread_t	Read_pthread;
static  void    *Read_thread_routine(void *);
static	void	Read_message(int fd, int code, void *data);
static void Connect(const char *port, const char *name, char *group);

static  mailbox Mbox;
static  char    Private_group[MAX_GROUP_NAME];
static  int     To_exit = 0;
static  void	Bye();

void Window::setup(){
    qDebug()<<"setup";
    Connect("4803","Ivan","me");
}


static void Connect(const char *port, const char *name, char *group){

    int	ret;
    sp_time test_timeout;

    test_timeout.sec = 5;
    test_timeout.usec = 0;

    ret = SP_connect_timeout( port, name, 0, 1, &Mbox, Private_group, test_timeout );

    if( ret != ACCEPT_SESSION )
    {
        SP_error( ret );
        Bye();
    }

    ret = SP_join(Mbox,group);
    qDebug()<<ret;
    if( ret != 0 )
    {
        SP_error( ret );
        Bye();
    }

//    E_init();
//    E_attach_fd( 0, READ_FD, User_command, 0, NULL, LOW_PRIORITY );
//    E_attach_fd( Mbox, READ_FD, Read_message, 0, NULL, HIGH_PRIORITY );

    ret = pthread_create( &Read_pthread, NULL, Read_thread_routine, 0 );
    qDebug("User: connected to %s with private group %s\n", port, Private_group);
}


static	void	Read_message(int fd, int code, void *data)
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
    printf("User> ");
    fflush(stdout);
}


static	void	*Read_thread_routine(void*)
{

    for(;;)
    {

        Read_message(0,0,NULL);
    }
    return( 0 );
}


static void sendMes(){
//    int ret;
    const char mess[11] = "test bagno";
    const char group[11] = "me";

    int m_len = strlen(mess);
    int num_groups = 1;
//    char	groups[10][MAX_GROUP_NAME];
    static const char* const groups[] = {"me"};
//    groups[0] = "me";
//    int ret= SP_multigroup_multicast( Mbox, SAFE_MESS, num_groups, (const char (*)[MAX_GROUP_NAME]) groups, 1, m_len, mess );
    int ret = SP_multicast(Mbox, SAFE_MESS, group, 1, m_len, mess);
    if( ret < 0 ) {
        SP_error( CONNECTION_CLOSED );
        Bye();
    }
    qDebug()<<ret;
}





static  void	Bye()
{
    To_exit = 1;
    printf("\nBye.\n");
    SP_disconnect( Mbox );
    pthread_join( Read_pthread, NULL );
    exit( 0 );
}


void Window::on_button_clicked()
{
    Bye();
}

void Window::on_pushButton_clicked()
{
    sendMes();
}
