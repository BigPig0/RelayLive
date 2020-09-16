#pragma once
#include <stdint.h>
#include <string>

namespace RtpDecode {
    /**
     * ����CRtpStreamʵ��������ʼ����udp
     * @param user CLiveWorkerʵ��
     * @param port ���ؼ�����udp�˿�
     */
    void* Creat(void* user, uint32_t port);

    /**
     * �����Ų�������CRtpStreamʵ��������ʼ����rtp
     * @param h CRtpStreamʵ��
     * @param sdp ���յ���Ӧ����Ϣ
     */
    void Play(void* h, std::string sdp);

    /**
     * ֹͣCRtpStream�Ľ��գ���ɾ��ʵ��
     * @param h CRtpStreamʵ��
     */
    void Stop(void* h);
};
