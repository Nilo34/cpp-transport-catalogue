// напишите решение с нуля
// код сохраните в свой git-репозиторий

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main () {
    transport_catalogue::TransportCatalogue TCatalogue;
    transport_catalogue::reader::Input(TCatalogue);
    transport_catalogue::request::EnquiryReader(TCatalogue);
}