#include <AvUtils/dataStructures/avStream.h>
#include <AvUtils/avMath.h>
#include <stdio.h>


struct AvStream avStreamCreate(void* buffer, uint64 size, AvFileDescriptor discard) {
    return (struct AvStream) {
        .buffer = buffer,
            .size = size,
            .pos = 0,
            .discard = discard
    };
}

void avStreamDiscard(AvStream stream) {
    if (stream->discard == AV_FILE_DESCRIPTOR_NULL) {
        return;
    }
    FILE* fp = fdopen(stream->discard, "w");
    if (fp){
        fwrite(stream->buffer, AV_MIN(stream->pos, stream->size),1,fp);
        fflush(fp);
    }
    stream->pos = 0;
}

void avStreamPutC(char chr, AvStream stream) {
    if (stream->pos >= stream->size) {
        if (stream->discard == 0) {
            return;
        }
        avStreamDiscard(stream);
    }
    
    stream->buffer[stream->pos++] = chr;
}

void avStreamFlush(AvStream stream) {
    avStreamDiscard(stream);
}