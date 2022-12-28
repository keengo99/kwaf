#ifndef KDSOAUTOBUFFER_H
#define KDSOAUTOBUFFER_H
#include <stdio.h>
#include "ksapi.h"
#include "kbuf.h"
extern kgl_dso_version *server_support;
#define DSO_BUFFER_SIZE 8192
class KAutoBufferData
{
public:
	kbuf *head;
	kbuf *last;
	char *hot;
	int total_len;
	int count;
	KREQUEST r;
};
class KDsoAutoBuffer : public KAutoBufferData
{
public:
public:
	KDsoAutoBuffer(KREQUEST r)
	{
		memset(static_cast<KAutoBufferData *>(this), 0, sizeof(KAutoBufferData));
		this->r = r;
	}
	~KDsoAutoBuffer()
	{
		clean();
	}
	void clean()
	{
		if (this->r == NULL) {
			destroy_kbuf(head);
			memset(static_cast<KAutoBufferData *>(this), 0, sizeof(KAutoBufferData));
		}
	}
	void writeSuccess(int got)
	{
		assert(last != NULL);
		last->used += got;
		total_len += got;
		hot += got;
	}
	char *getWriteBuffer(int &len)
	{
		if (last == NULL) {
			assert(head == NULL);
			head = new_buff(DSO_BUFFER_SIZE);
			last = head;
			hot = head->data;
		}
		len = DSO_BUFFER_SIZE - last->used;
		if (len == 0 || hot == NULL) {
			kbuf *nbuf = new_buff(DSO_BUFFER_SIZE);
			assert(last->next == NULL);
			last->next = nbuf;
			last = nbuf;
			hot = last->data;
			len = DSO_BUFFER_SIZE;
		}
		assert(len > 0);
		return hot;
	}
	int write(const char *buf, int len)
	{
		int wlen;
		char *t = getWriteBuffer(wlen);
		assert(t);
		wlen = KGL_MIN(len, wlen);
		kgl_memcpy(t, buf, wlen);
		writeSuccess(wlen);
		return wlen;
	}
	bool write_all(const char *buf, int len)
	{
		while (len > 0) {
			int r = write(buf, len);
			if (r <= 0)
				return false;
			len -= r;
			buf += r;
		}
		return true;
	}
	void Insert(const char *str, int len) {
		kbuf *buf = new_buff(len);
		kgl_memcpy(buf->data, str, len);
		buf->used = len;
		Insert(buf);
	}
	inline void Insert(kbuf *buf)
	{
		buf->next = head;
		if (last == NULL) {
			last = buf;
		}
		head = buf;
		total_len += buf->used;
	}
	inline void Append(kbuf *buf)
	{
		if (last == NULL) {
			kassert(head == NULL);
			head = buf;
		} else {
			assert(head);
			last->next = buf;
		}
		buf->next = NULL;
		hot = NULL;
		last = buf;
		total_len += buf->used;
	}
	void SwitchRead()
	{
		if (head) {
			hot = head->data;
		}
	}
	inline void print()
	{
		kbuf *tmp = head;
		while (tmp) {
			if (tmp->used > 0) {
				fwrite(tmp->data, 1, tmp->used, stdout);
			}
			tmp = tmp->next;
		}
	}
	unsigned getLen()
	{
		return total_len;
	}
	kbuf *getHead()
	{
		return head;
	}
	kbuf *stealBuffFast()
	{
		kbuf *ret = head;
		head = last = NULL;
		hot = NULL;
		return ret;
	}
	inline WSABUF *GetReadBuffer(int *bc)
	{
		*bc = count;
		if (count == 0) {
			return NULL;
		}
		WSABUF *buf;
		if (r == NULL) {
			buf = (WSABUF *)xmalloc(sizeof(WSABUF)*count);
		} else {
			buf = (WSABUF *)server_support->f->alloc_memory(r, sizeof(WSABUF)*count, KF_ALLOC_REQUEST);
		}
		assert(head);
		kbuf *tmp = head;
		int i;
		for (i = 0; i < count; i++) {
			buf[i].iov_base = tmp->data;
			buf[i].iov_len = tmp->used;
			tmp = tmp->next;
			if (tmp == NULL) {
				break;
			}
		}
		return buf;
	}
	inline KDsoAutoBuffer & operator <<(const char *str)
	{
		if (!write_all(str, (int)strlen(str))) {
			fprintf(stderr, "cann't write to stream\n");
		}
		return *this;
	}
	inline KDsoAutoBuffer & operator <<(const int value) {
		char buf[16];
		int len = snprintf(buf, 15, "%d", value);
		if (len <= 0) {
			return *this;
		}
		if (!write_all(buf, len)) {
			fprintf(stderr, "cann't write to stream\n");
		}
		return *this;
	}
	inline kbuf *new_buff(int chunk_size)
	{
		count++;
		if (r == NULL) {
			kbuf *buf = (kbuf *)xmalloc(sizeof(kbuf));
			buf->flags = 0;
			buf->data = (char *)xmalloc(chunk_size);
			buf->used = 0;
			buf->next = NULL;
			return buf;
		}
		kbuf *buf = (kbuf *)server_support->f->alloc_memory(r, sizeof(kbuf), KF_ALLOC_REQUEST);
		buf->flags = 0;
		buf->data = (char *)server_support->f->alloc_memory(r, chunk_size, KF_ALLOC_REQUEST);
		buf->used = 0;
		buf->next = NULL;
		return buf;
	}
};
#endif

