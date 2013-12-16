;;; emacstts.el --- A simple TTS engine for Emacs

;;; Copyright (C) 2013 by Henry Wang <wh_henry@hotmail.com>

;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 2, or (at your option)
;;; any later version.

;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.

;;; You should have received a copy of the GNU General Public License
;;; along with this program; see the file COPYING.  If not, write to the
;;; Free Software Foundation, Inc.,   51 Franklin Street, Fifth Floor,
;;; Boston, MA  02110-1301  USA

;;; Commentary:
;;;
;;; Load this file and run:
;;;
;;;   M-x emacstts-say
;;;
;;; to select a region and run
;;;
;;;   M-x emacstts-say-region
;;;
;;; to speak out just the region.
;;;


;;; Code:
(require 'epc)

;;;###autoload
(defcustom emacstts-py-script (locate-file "emacstts.py" load-path)
    "The full path name of the emacstts.py script."
    :type 'string
    :group 'emacstts)

;;;###autoload
(defvar emacstts-epc (epc:start-epc "python" (list emacstts-py-script)))

;;;###autoload
(defun emacstts-destroy()
  "Destruct the epc connection"
  (interactive)
  (epc:stop-epc emacstts-epc)
)

;;;###autoload
(defun emacstts-say(text)
  "Read the supplied text"
  (interactive "sText: ")
  (message "Saying %s" text)
  (epc:call-sync emacstts-epc 'say (list text))
)

;;;###autoload
(defun emacstts-say-region(start end)
  "Read the text in region bounded by start and end"
  (interactive "r")
  (emacstts-say (buffer-substring-no-properties start end))
)

;;;###autoload
(defun emacstts-shutup()
  "Interrupt the current speaking"
  (interactive)
  (epc:call-sync emacstts-epc 'shutup '())
)

;;;###autoload
(defun emacstts-louder()
  "Increate the volume"
  (interactive)
  (epc:call-sync emacstts-epc 'louder '())
)

;;;###autoload
(defun emacstts-quieter()
  "Reduce the volume"
  (interactive)
  (epc:call-sync emacstts-epc 'quieter '())
)

;;;###autoload
(defun emacstts-faster()
  "Increate the speaking speed"
  (interactive)
  (epc:call-sync emacstts-epc 'faster '())
)

;;;###autoload
(defun emacstts-slower()
  "Reduce the speaking speed"
  (interactive)
  (epc:call-sync emacstts-epc 'slower '())
)

;;;###autoload
(defun emacstts-voices()
  "Return the available voices in the current TTS engine"
  (epc:call-sync emacstts-epc 'voices '())
)

;;;###autoload
(defun emacstts-list-voices()
  "Print the available voices in the current TTS engine"
  (interactive)
  (with-output-to-temp-buffer "*emacstts voices*"
    (let (voices)
      (setq voices (emacstts-voices))
      (while voices
        (print (pop voices))))
    (switch-to-buffer "*emacstts voices*"))
)

;;;###autoload
(defun emacstts-setvoice(name)
  "Set the TTS voice to supplied one"
  (interactive
   (list (completing-read "voice: " (emacstts-voices))))
  (message "new voice %s" name)
  (epc:call-sync emacstts-epc 'setvoice (list name))
)

(provide 'emacstts)
;;; emacstts.el ends here
