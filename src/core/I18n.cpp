#include "I18n.h"
#include "Settings.h"

I18n& I18n::instance() {
    static I18n i;
    return i;
}

I18n::I18n() {
    m_language = Settings::instance().language();
    m_table = {
        {"app_name", {"SaveGuard", "SaveGuard"}},
        {"tagline", {"Localizador de duplicatas e espaço em disco", "Duplicate finder and disk space cleaner"}},
        {"nav_dashboard", {"Painel", "Dashboard"}},
        {"nav_duplicates", {"Duplicatas", "Duplicates"}},
        {"nav_bigfiles", {"Arquivos grandes", "Large files"}},
        {"nav_history", {"Histórico", "History"}},
        {"nav_settings", {"Configurações", "Settings"}},
        {"state_idle", {"Pronto", "Ready"}},
        {"state_scanning", {"Analisando", "Scanning"}},
        {"state_paused", {"Pausado", "Paused"}},
        {"state_canceled", {"Análise cancelada", "Scan canceled"}},
        {"state_done", {"Análise concluída", "Scan completed"}},
        {"state_working", {"Operação em andamento", "Operation in progress"}},
        {"scan_state_enumerate", {"Percorrendo pastas", "Walking folders"}},
        {"scan_state_hash", {"Calculando hashes: %1 de %2", "Hashing: %1 of %2"}},
        {"scan_state_group", {"Agrupando resultados", "Grouping results"}},
        {"scan_state_busy", {"Já existe uma análise em andamento", "A scan is already running"}},
        {"btn_start_scan", {"Analisar", "Scan"}},
        {"btn_pause", {"Pausar", "Pause"}},
        {"btn_resume", {"Retomar", "Resume"}},
        {"btn_cancel_scan", {"Cancelar", "Cancel"}},
        {"btn_add_folder", {"Adicionar pasta", "Add folder"}},
        {"btn_remove_folder", {"Remover selecionada", "Remove selected"}},
        {"btn_open_file", {"Abrir arquivo", "Open file"}},
        {"btn_open_location", {"Abrir local", "Open location"}},
        {"btn_review", {"Revisar e enviar para a Lixeira", "Review and move to trash"}},
        {"btn_send_trash", {"Mover para a Lixeira", "Move to trash"}},
        {"btn_clear", {"Limpar", "Clear"}},
        {"btn_save", {"Salvar", "Save"}},
        {"btn_refresh", {"Atualizar", "Refresh"}},
        {"btn_close", {"Fechar", "Close"}},
        {"btn_rescan", {"Analisar novamente", "Scan again"}},
        {"btn_scan_big", {"Encontrar arquivos", "Find files"}},
        {"dash_drives", {"Unidades de disco", "Disk drives"}},
        {"dash_drive", {"Unidade", "Drive"}},
        {"dash_fs", {"Sistema", "Filesystem"}},
        {"dash_total", {"Total", "Total"}},
        {"dash_used", {"Usado", "Used"}},
        {"dash_free", {"Livre", "Free"}},
        {"dash_use", {"Uso", "Usage"}},
        {"dash_files", {"Arquivos analisados", "Files analyzed"}},
        {"dash_groups", {"Grupos de duplicatas", "Duplicate groups"}},
        {"dash_recoverable", {"Espaço recuperável", "Recoverable space"}},
        {"dash_last_scan", {"Última análise", "Last scan"}},
        {"dash_never", {"Nenhuma análise executada ainda", "No scan has run yet"}},
        {"dash_shortcuts", {"Atalhos", "Shortcuts"}},
        {"dash_recent", {"Operações recentes", "Recent operations"}},
        {"dash_errors_line", {"%1 erro(s) durante a análise", "%1 error(s) during the scan"}},
        {"dup_folders", {"Pastas a analisar", "Folders to scan"}},
        {"dup_folders_hint", {"Adicione uma ou mais pastas. Diretórios do sistema são ignorados automaticamente.", "Add one or more folders. System directories are skipped automatically."}},
        {"dup_empty", {"Nenhuma análise executada. Adicione pastas e clique em Analisar.", "No scan yet. Add folders and click Scan."}},
        {"dup_running", {"Análise em andamento. As ações destrutivas ficam bloqueadas até o fim.", "Scan in progress. Destructive actions stay locked until it finishes."}},
        {"dup_summary_groups", {"%1 grupo(s) · %2 arquivo(s) selecionado(s) · %3 recuperável(is)", "%1 group(s) · %2 file(s) selected · %3 recoverable"}},
        {"dup_none_found", {"Nenhuma duplicata encontrada nas pastas informadas.", "No duplicates found in the selected folders."}},
        {"dup_col_file", {"Arquivo", "File"}},
        {"dup_col_size", {"Tamanho", "Size"}},
        {"dup_col_mtime", {"Modificado em", "Modified"}},
        {"dup_col_action", {"Remover", "Remove"}},
        {"dup_group_label", {"Duplicatas · %1 · recuperável %2", "Duplicates · %1 · recoverable %2"}},
        {"dup_ctx_keep", {"Manter esta cópia", "Keep this copy"}},
        {"dup_ctx_remove", {"Marcar para remover", "Mark for removal"}},
        {"dup_at_least_one", {"Em cada grupo, ao menos uma cópia deve ser mantida.", "At least one copy in each group must be kept."}},
        {"big_threshold", {"Tamanho mínimo", "Minimum size"}},
        {"big_custom", {"Personalizado", "Custom"}},
        {"big_search", {"Pesquisar por nome", "Search by name"}},
        {"big_ext_all", {"Todas as extensões", "All extensions"}},
        {"big_empty", {"Nenhuma análise executada. Escolha o tamanho mínimo e clique em Encontrar arquivos.", "No scan yet. Pick a minimum size and click Find files."}},
        {"big_none_found", {"Nenhum arquivo acima do limite informado.", "No files above the limit."}},
        {"big_selected", {"%1 arquivo(s) selecionado(s) · %2", "%1 file(s) selected · %2"}},
        {"big_col_name", {"Nome", "Name"}},
        {"big_col_path", {"Local", "Path"}},
        {"big_col_size", {"Tamanho", "Size"}},
        {"big_col_mtime", {"Modificado em", "Modified"}},
        {"big_col_ext", {"Extensão", "Extension"}},
        {"review_title", {"Central de revisão", "Review center"}},
        {"review_subtitle", {"Os arquivos abaixo serão movidos para a Lixeira do sistema. Nada é excluído permanentemente pelo SaveGuard.", "The files below will be moved to the system trash. SaveGuard never deletes permanently."}},
        {"review_col_path", {"Caminho", "Path"}},
        {"review_col_size", {"Tamanho", "Size"}},
        {"review_total", {"Total: %1 arquivo(s) · %2", "Total: %1 file(s) · %2"}},
        {"review_validate_note", {"Antes de mover, cada arquivo é revalidado (tamanho, data e conteúdo). Arquivos alterados ou protegidos são preservados.", "Each file is revalidated before moving (size, date, content). Changed or protected files are preserved."}},
        {"review_empty", {"Selecione ao menos um arquivo antes de revisar.", "Select at least one file before reviewing."}},
        {"hist_col_date", {"Data", "Date"}},
        {"hist_col_dur", {"Duração", "Duration"}},
        {"hist_col_type", {"Tipo", "Type"}},
        {"hist_col_roots", {"Pastas", "Folders"}},
        {"hist_col_files", {"Arquivos", "Files"}},
        {"hist_col_rec", {"Recuperável", "Recoverable"}},
        {"hist_col_trashed", {"Enviados", "Trashed"}},
        {"hist_col_errors", {"Erros", "Errors"}},
        {"hist_empty", {"Sem registros ainda. Análises e operações aparecem aqui.", "No records yet. Scans and operations appear here."}},
        {"hist_details", {"Detalhes", "Details"}},
        {"hist_clear_title", {"Apagar histórico", "Clear history"}},
        {"hist_clear_text", {"Apagar todos os registros de análises e operações?", "Delete all scan and operation records?"}},
        {"hist_type_scan", {"Análise", "Scan"}},
        {"hist_type_cleanup", {"Limpeza", "Cleanup"}},
        {"hist_type_scan_big", {"Análise (grandes)", "Scan (large)"}},
        {"set_dirs", {"Pastas ignoradas", "Ignored folders"}},
        {"set_dirs_hint", {"Um caminho ou nome de pasta por linha. A pasta e todo o conteúdo são excluídos da análise.", "One path or folder name per line. The folder and its contents are excluded from scans."}},
        {"set_exts", {"Extensões ignoradas", "Ignored extensions"}},
        {"set_exts_hint", {"Uma extensão por linha, por exemplo .tmp", "One extension per line, for example .tmp"}},
        {"set_min_size", {"Tamanho mínimo por arquivo", "Minimum size per file"}},
        {"set_concurrency", {"Tarefas simultâneas de hash", "Concurrent hash tasks"}},
        {"set_language", {"Idioma", "Language"}},
        {"set_lang_pt", {"Português (Brasil)", "Portuguese (Brazil)"}},
        {"set_lang_en", {"English", "English"}},
        {"set_saved", {"Configurações salvas.", "Settings saved."}},
        {"set_protected_note", {"Diretórios críticos do sistema (Windows, Program Files, /usr, /etc e similares) nunca são analisados nem removidos.", "Critical system directories (Windows, Program Files, /usr, /etc and similar) are never scanned or removed."}},
        {"set_trash_note", {"A remoção usa a Lixeira do sistema. Se o sistema não oferecer suporte, a operação falha e o arquivo permanece intacto. O SaveGuard não garante restauração de itens já enviados à Lixeira.", "Removal uses the system trash. Without trash support the operation fails and the file stays intact. SaveGuard does not guarantee restoration of items already trashed."}},
        {"cleanup_title", {"Resultado da limpeza", "Cleanup result"}},
        {"cleanup_ok_line", {"%1 arquivo(s) enviados à Lixeira · %2 liberados", "%1 file(s) moved to trash · %2 freed"}},
        {"cleanup_fail_line", {"%1 falha(s). Os arquivos abaixo permanecem intactos.", "%1 failure(s). The files below remain intact."}},
        {"cleanup_err_missing", {"Arquivo não existe mais (movido ou apagado por outro processo)", "File no longer exists (moved or deleted by another process)"}},
        {"cleanup_err_symlink", {"É um link simbólico; links não são removidos", "It is a symbolic link; links are never removed"}},
        {"cleanup_err_protected", {"Caminho protegido pelo sistema", "Protected system path"}},
        {"cleanup_err_changed_size", {"Tamanho mudou desde a análise", "Size changed since the scan"}},
        {"cleanup_err_changed_mtime", {"Data de modificação mudou desde a análise", "Modification date changed since the scan"}},
        {"cleanup_err_changed_content", {"Conteúdo mudou desde a análise", "Content changed since the scan"}},
        {"cleanup_err_read", {"Falha ao reler o arquivo para revalidação", "Could not reread the file for revalidation"}},
        {"cleanup_err_trash", {"A Lixeira recusou o item; o arquivo permanece onde está", "The trash refused the item; the file stays where it is"}},
        {"cleanup_canceled_line", {"Operação cancelada; itens restantes intactos.", "Operation canceled; remaining items untouched."}},
        {"err_no_folder", {"Adicione ao menos uma pasta antes de analisar.", "Add at least one folder before scanning."}},
        {"err_root_missing", {"Pasta não encontrada: %1", "Folder not found: %1"}},
        {"fmt_duration", {"%1 min %2 s", "%1 min %2 s"}},
        {"fmt_duration_ms", {"%1 ms", "%1 ms"}},
        {"fmt_files", {"%1 arquivo(s)", "%1 file(s)"}},
        {"fmt_groups", {"%1 grupo(s)", "%1 group(s)"}},
        {"fmt_rows", {"%1 linha(s)", "%1 row(s)"}},
    };
}

QString I18n::t(const QString& key) const {
    const auto it = m_table.constFind(key);
    if (it == m_table.constEnd()) return key;
    return isEnglish() ? it.value().second : it.value().first;
}

void I18n::setLanguage(const QString& lang) {
    if (m_language == lang) return;
    m_language = lang;
    Settings::instance().setLanguage(lang);
    Settings::instance().save();
    emit languageChanged();
}

QString I18n::formatBytes(qint64 bytes) {
    if (bytes < 0) bytes = 0;
    const double kb = 1024.0;
    if (bytes < kb) return QString::number(bytes) + QStringLiteral(" B");
    if (bytes < kb * kb) return QString::number(bytes / kb, 'f', 1) + QStringLiteral(" KB");
    if (bytes < kb * kb * kb) return QString::number(bytes / (kb * kb), 'f', 1) + QStringLiteral(" MB");
    return QString::number(bytes / (kb * kb * kb), 'f', 2) + QStringLiteral(" GB");
}
