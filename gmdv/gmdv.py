#!/usr/bin/env python

import imaplib
import email
import re, os, stat, time
import errno
import fuse

fuse.fuse_python_api = (0, 2)

class GmailFS(fuse.Fuse):
    def __init__(self, *args, **kw):
        fuse.Fuse.__init__(self, *args, **kw)
        self.gmail = {}
        self.conn = None
        self.open_gmail()

    def parse_mbox(self, boxname):
        mails = []
        self.conn.select(boxname)
        r, data = self.conn.uid('search', None, 'ALL')
        for mid in data[0].split():
            r, m = self.conn.uid('fetch', mid, '(RFC822)')
            msg = email.message_from_string(m[0][1])
            self.dump_mail(msg)
            mails.append(self.parse_mail(mid, msg))
        return mails
        
    def dump_mail(self, m):
        print '-------------------------------------------------------'
        for part in m.walk():
            print part.get_content_type()
            if not part.is_multipart():
                print part.get_payload(decode=True)

    def parse_mail(self, mid, msg):
        mail = {}
        mail['id'] = mid
        mail['to'] = msg['To']
        mail['from'] = email.utils.parseaddr(msg['From'])
        mail['subject'] = msg['Subject']
        mail['date'] = msg['Date']
        mail['fname'] = self.format_mail_file(mail['id'], mail['subject'])
        mail['html'] = False
        mail['body'] = ''
        maintype = msg.get_content_maintype()
        if maintype == 'multipart':
            for part in msg.get_payload():
                if part.get_content_maintype() == 'text':
                    if part.get_content_subtype() == 'html':
                        mail['body'] = part.get_payload(decode=True)
                        mail['html'] = True
                        break
                    elif len(mail['body']) == 0:
                        mail['body'] = msg.get_payload()
        elif maintype == 'text':
            mail['body'] = msg.get_payload()
        return mail

    def format_mail_body(self, mail):
        if mail['html']:
            return '<!--\r\nSUBJECT: %s\r\nFROM: %s\r\nTO: %s\r\nDATE: %s\r\n-->\r\n%s' \
                %(mail['subject'], mail['from'], mail['to'], mail['date'], mail['body'])
        else:
            return 'SUBJECT: %s\r\nFROM: %s\r\nTO: %s\r\nDATE: %s\r\n%s' \
                %(mail['subject'], mail['from'], mail['to'], mail['date'], mail['body'])

    def format_mail_file(self, mid, subject):
        invalid_chars = [' ', '/', '\\', '?', '%', '*', ':', '|', '"', '<', '>', '.']
        s = '%s-%s' %(mid.zfill(5), subject)
        for c in invalid_chars:
            s = s.replace(c, '_')
        return s + '.gm'

    def get_passwd(self):
        pf = open('pass.txt', 'r')
        line = pf.readline().strip()
        pf.close()
        return line

    def open_gmail(self):
        lstpattern = re.compile(r'\((?P<flags>.*?)\) "(?P<delimiter>.*)" (?P<name>.*)')
        self.conn = imaplib.IMAP4_SSL('imap.gmail.com')
        self.conn.login('henry118@gmail.com', self.get_passwd())
        r, data = self.conn.list()
        for line in data:
            flags, delimiter, mailbox_name = lstpattern.match(line).groups()
            bn = mailbox_name.strip('"')
            #if bn in ['INBOX', '[Gmail]/Drafts', '[Gmail]/Spam', '[Gmail]/Trash']:
            if bn in ['INBOX', '[Gmail]/Drafts', '[Gmail]/Trash']:
                mailbox = {}
                mailbox['name'] = mailbox_name
                mailbox['timestamp'] = time.time()
                mailbox['content'] = self.parse_mbox(mailbox_name)
                l = bn.split('/')
                if len(l) == 1:
                    mailbox['display'] = bn.lower()
                else:
                    mailbox['display'] = l[1].lower()
                self.gmail[mailbox['display']] = mailbox

    def close_gmail(self):
        if self.conn != None:
            self.conn.logout()

    def getmail(self, boxname, fname):
        if not boxname in self.gmail:
            return None
        mailbox = self.gmail[boxname]
        for m in mailbox['content']:
            if m['fname'] == fname:
                return m
        return None
        
    def delmail(self, boxname, fname):
        m = self.getmail(boxname, fname)
        if m != None:
            mails = self.gmail[boxname]['content']
            mails.remove(m)

    def getattr(self, path):
        print '> getattr', path
        st = fuse.Stat()
        st.st_mode = stat.S_IFDIR | 0700
        st.st_uid = os.getuid()
        st.st_gid = os.getgid()
        st.st_nlink = 2
        st.st_atime = int(time.time())
        st.st_mtime = st.st_atime
        st.st_ctime = st.st_atime
        pe = path.split('/')[1:]

        if path == '/':
            pass
        elif pe[-1] in self.gmail.keys():
            pass
        elif len(pe) > 1 and self.getmail(pe[0], pe[1]) != None:
            m = self.getmail(pe[0], pe[1])
            st.st_mode = stat.S_IFREG | 0600
            st.st_nlink = 1
            st.st_size = len(self.format_mail_body(m))
            #st.st_mtime = int(time.mktime(time.strptime(m['date'][0:-6], '%a, %d %b %Y %H:%M:%S')))
        else:
            return - errno.ENOENT
        return st

    def readdir(self, path, offset):
        print '> readdir', path, offset
        dirents = [ '.', '..' ]
        pe = path.split('/')[1]
        if path == '/':
            for k in self.gmail.keys():
                dirents.append(k)
        elif pe in self.gmail:
            mailbox = self.gmail[pe]
            now = time.time()
            last = mailbox['timestamp']
            if last == None or now - last > 60: # 60 seconds
                mailbox['timestamp'] = now
                mailbox['content'] = self.parse_mbox(mailbox['name'])
            for m in mailbox['content']:
                dirents.append(m['fname'])
        for r in dirents:
            yield fuse.Direntry(r)

    def read(self, path, size, offset):
        print '> read', path, size, offset
        pe = path.split('/')[1:]
        m = self.getmail(pe[0], pe[1])
        if m != None:
            s = self.format_mail_body(m)
            return s[offset:offset+size]
        return 0

    def mknod(self, path, mode, dev):
        print '> mknod'
        return -errno.EACCES

    def unlink(self, path):
        print '> unlink', path
        pe = path.split('/')[1:]
        if path == '/' or not pe[0] in self.gmail:
            return -errno.EACCES
        mb = self.gmail[pe[0]]
        m = self.getmail(pe[0], pe[1])
        if m == None:
            return -errno.EINVAL
        self.conn.select(mb['name'])
        ret, msg = self.conn.uid('fetch', m['id'], '(FLAGS)')
        print 'Flags before:', msg
        ret, msg = self.conn.uid('store', m['id'], '+FLAGS', '(\\Deleted)')
        print ret, msg
        ret, msg = self.conn.uid('fetch', m['id'], '(FLAGS)')
        print 'Flags after:', msg
        ret, msg = self.conn.expunge()
        print ret, msg
        if ret == 'OK' and msg != None and msg[0] != None:
            self.delmail(pe[0], m['fname'])
        return 0

    def rename(self, pathfrom, pathto):
        print '> rename', pathfrom, pathto
        """
        fpe = pathfrom.split('/')[1:]
        tpe = pathto.split('/')[1:]
        if not fpe[0] in self.gmail or not tpe[0] in self.gmail:
            return -errno.EINVAL
        m = self.getmail(fpe[0], pe[1])
        if m == None:
            return -errno.EINVAL
        tmb = self.gmail[tpe[0]]
        self.conn.uid('copy', m['id'], tmb['name'])
        """
        return -errno.EACCES

    def write(self, path, buf, offset):
        print '> write', path, buf, offset
        return -errno.EACCES

    def release(self, path, flags):
        print '> release', path
        return 0

    def open(self, path, flags):
        print '> open', path
        return 0

    def truncate(self, path, size):
        print '> truncate', path, size
        return -errno.EACCES

    def utime(self, path, times):
        print '> utime', path, times
        return 0

    def mkdir(self, path, mode):
        print '> mkdir', path
        return -errno.EACCES

    def rmdir(self, path, mode):
        print '> rmdir', path
        return -errno.EACCES

    def fsync(self, path, isfsyncfile):
        print '> fsync', path, isfysncfile
        return 0

def main():
    fs = GmailFS(dash_s_do='setsingle')
    fs.parse(errex=1)
    print 'Ready...'
    fs.main()
    fs.close_gmail()

if __name__ == '__main__':
    main()
